
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>


typedef int bool;
#define true   (1)
#define false  (0)

//maximum number of loops while trying to erase flash memory
#define MAXLOOPS 10

#define RANGE_REG 0x00800000
#define FLASHBASE 0x00c00000
#define RANGELIMIT  0x100000
#define MAXLEN    132
#define BS    19
#define RANGENUMBER 4 //using memory blocks 4 and 5 instead of (0 and 1) or (2 and 3)

#define VME_LENGTH      0x01000000
#define MODULE_LENGTH 0x01000000
#define MINCLOCKS 300000

//For KPH VMEbus PC
#define VMEBUSPC_EXTOFFSET  0x80000000
// Size of VME-segment in bits: 4096 = 2^12 = 0x1000
const unsigned long VME_SegSize =  0x1000;


int vme_fd=-1;
void *vmevirtbase;
size_t vmelaenge;
// for logging
FILE *logFile;
int timestamp;
bool quiet;


sighandler(l_signal) int l_signal;
{
  switch(l_signal) {
  case SIGKILL:
    printf("signal SIGKILL received \n");
    break;
  case SIGINT:
    printf("signal SIGINT received \n");
    break;
  case SIGQUIT:
    printf("signal SIGQUIT received \n");
    break;
  case SIGTERM:
    printf("signal SIGTERM received \n");
    break;
  case SIGBUS:
    printf("signal SIGBUS received \n");
    break;
  case SIGSEGV:
    printf("signal SIGSEGV received \n");
    break;
  default:
    printf("signal %d received \n",l_signal);
    break;
  }
  //  return_controller(vme_virt_addr,vme_len);
  exit(0);
}

void printlog(char *output, bool newline, bool verbose)
{
  if (logFile != NULL) {
    if (newline) {
      fprintf(logFile, "\n");
    }
    if (newline) {
      fprintf(logFile, "[%08d] ", (int) time(NULL) - timestamp);
    }
    fprintf(logFile, output);
  }
  if (verbose) {
    if (newline) {
      printf("\n");
    }
    printf("%s", output);
    fflush(stdout);
  }
  //verbose...
}

void printlogInt(int number, bool verbose)
{
  if (logFile != NULL) {
    fprintf(logFile, "%d [%#x]", number, number);
  }
  if (verbose) {
    printf("%d [%#x]", number, number);
    fflush(stdout);
  }
}

void openLog(bool verbose)
{
  if((logFile = fopen("vuprom.log" ,"a")) == (FILE *)NULL) {
    printlog("warning: unable to open log file", true, verbose);
  }
  printlog("Program started, scanning commandline", true, verbose);
}

void closeLog(bool verbose)
{
  if (logFile != NULL) {
    printlog("stopping logging\n", true, verbose);
    close(logFile);
  }
}


unsigned long openvme(void *vmeaddr,size_t vme_len, bool verbose)
{
  printlog("opening device", true, verbose);

  //** Fill the upper 3 bits
  //    const int vme_regA32_fd = open("/dev/mem", O_RDWR);
  int vme_regA32_fd = open("/dev/mem", O_RDWR);
  if( vme_regA32_fd == -1 ) {
    printf("error: open #1 /dev/mem failed, exiting ...\n");
    exit(-1);
  }

  // ... and map the part starting at offset 0xaa000000 to memory (with
  // VME seg.size as minimal size, even if it's only one register to access).
  //
  // definition:
  // void * mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
  // See: http://www.freebsd.org/cgi/man.cgi?query=mmap
  volatile void *const reg_A32_mmap
  = mmap(NULL, VME_SegSize, (PROT_READ | PROT_WRITE), MAP_SHARED, vme_regA32_fd, 0xaa000000);

  if( reg_A32_mmap == MAP_FAILED ) {
    printf("error: mmap #1 failed, exiting ...\n");
    exit(-1);
  }

  // Now write the upper three bits of the hardware base address into
  // this special register.
  unsigned long NewAddr, UpperAddr, LowerAddr;
  NewAddr = (unsigned long)vmeaddr;
  UpperAddr = NewAddr & 0xe0000000;
  LowerAddr= NewAddr & 0x1fffffff;
  *((unsigned long *)reg_A32_mmap) = UpperAddr ;

  //** Fill the upper 3 bits End




  vme_fd = open("/dev/mem", O_RDWR);
  if( vme_fd == -1 ) {
    printlog("Error opening vme device, aborting", true, verbose);
    closeLog(verbose);
    exit(1);
  }
  //  vmevirtbase  = mmap(NULL, VME_LENGTH, PROT_READ|PROT_WRITE, MAP_SHARED, vme_fd, 0x10000000 | VMEBUSPC_EXTOFFSET); //(off_t) vmeaddr
  //  vmevirtbase  = mmap(NULL, VME_LENGTH, PROT_READ|PROT_WRITE, MAP_SHARED, vme_fd, ((unsigned long) vmeaddr) | VMEBUSPC_EXTOFFSET); //(off_t) vmeaddr
  vmevirtbase  = mmap(NULL, VME_LENGTH, PROT_READ|PROT_WRITE, MAP_SHARED, vme_fd, LowerAddr | VMEBUSPC_EXTOFFSET); //(off_t) vmeaddr
  if (vmevirtbase==MAP_FAILED) {
    printlog("memory mapping failed, aborting", true, verbose);
    closeLog(verbose);
    exit(1);
  }
  printlog("vme virtual base is located at: ", true, verbose);
  printlogInt((unsigned long) vmevirtbase, verbose);
  printlog("device open", true, verbose);
  return (unsigned long) vmevirtbase;
}

void closevme(bool verbose)
{
  printlog("closing vme device ... ", true, verbose);
  munmap((void *)vmevirtbase,vmelaenge);
  close(vme_fd);
  printlog("done", false, verbose);
}

int select_range(unsigned long base, int range, bool verbose)
{
  unsigned long *reg;
  unsigned long val;
  printlog("setting range to: ", true, verbose);
  printlogInt(range, verbose);
  printlog(" at adress: ", false, verbose);
  printlogInt(base + RANGE_REG, verbose);
  reg = (unsigned long *)(base + RANGE_REG);
  val = range & 0x7; // if range = 0 this means: using Memory block 0 and 1

  *reg = val;
  usleep(100);
  val = *reg;

  printlog("set range returned: ", true, verbose);
  printlogInt(val, verbose);
  return (val);
}

int erase_block(int base, int block, bool verbose)
{
  unsigned long clockCounter;
  volatile int *addr;
  addr =  (volatile int *)(base + (block<<BS));


  *addr = 0x20;
  *addr = 0xd0;

  while((*addr & 0x80) == 0) {
    ;
  }
  //    if (verbose)  printf("addr = %x\n",*addr);
  if((*addr & 0x40) ==0) {
    //      printf("addr = %x\n",*addr);
    return(0);
  } else {
    return(1);
  }

}

int prog_flash(int faddr, int data, bool verbose)
{
  volatile int *addr;

  //  printf("prog addr %x data %x \n",faddr,data);
  addr = (volatile int *)( faddr);
  *addr = 0x40;
  *addr = data;
  while((*addr & 0x80) == 0) {
    ;
  }

  //  printf("status = %x\n",read_status(faddr));
  //  while((read_status(faddr) & 0x80) == 0);


}

void read_array(int base)
{
  volatile int *addr;

  addr = (volatile int *)(base );

  *addr = 0xff;
}

void restart(int base,int range)
{
  volatile int *reg;
  int d;

  reg = (volatile int *)(base + RANGE_REG);
  d = (range & 0x7) + 0x8;
  *reg = d;
}

void eraseFlashMemory(int moduleNumber, bool simulate, bool verbose)
{
  int i;
  int status;
  bool errorOccured = true;
  int loopCounter = 0;
  unsigned long vmeaddr;
  unsigned long vme_virt_addr;
  int fbase;

  printlog("erasing flash memory", true, !quiet);
  while (errorOccured && (loopCounter < MAXLOOPS)) {
    loopCounter++;
    printlog("attempt #: ", true, verbose);
    printlogInt(loopCounter, verbose);
    errorOccured = false;
    vmeaddr = moduleNumber * MODULE_LENGTH  ;
    vme_virt_addr = (unsigned long) openvme((void *)vmeaddr, VME_LENGTH, verbose);
    fbase = vme_virt_addr + FLASHBASE;
    select_range(vme_virt_addr, 0+RANGENUMBER, verbose);
    for (i = 0; i <= 7; i++) {
      printlog("block #: ", true, verbose);
      printlogInt(i, verbose);
      if (!simulate) {
        status = erase_block(fbase, i, verbose);
      } else {
        status = 0;
      }
      if (status == 0) {
        printlog(" successfully erased", false, verbose);
      } else {
        printlog(" can\'t be erased", false, verbose);
        errorOccured = true;
      }
    }

    if (!errorOccured) {
      select_range(vme_virt_addr, 1+RANGENUMBER, verbose);
      for (i = 0; i <= 7; i++) {
        printlog("block #: ", true, verbose);
        printlogInt(i, verbose);
        if (!simulate) {
          status = erase_block(fbase, i, verbose);
        } else {
          status = 0;
        }
        if (status == 0) {
          printlog(" successfully erased", false, verbose);
        } else {
          printlog(" can\'t be erased", false, verbose);
          errorOccured = true;
        }
      }
    }
    read_array(fbase);
    closevme(verbose);
    sleep(2);
  }
  if(errorOccured) {
    printlog("Error erasing flash memory, aborting", true, verbose);
    closeLog(verbose);
    exit(1);
  }

}

void writeFlashMemory(int moduleNumber, FILE *rbtFile, bool simulate, bool verbose)
{
  int i, j;
  int len;
  int l_word;
  int bits = 0;
  char c_input[MAXLEN];
  volatile int *p_addr;
  int l_count = 0;
  unsigned long vmeaddr, vme_virt_addr;
  int fbase, line;
  line = 0;
  printlog("writing new program to flash memory", true, !quiet);

  vmeaddr = moduleNumber * MODULE_LENGTH  ;
  //    vme_len = 0x1000000;
  //    vme_am = 0x09;

  vme_virt_addr = (unsigned long)openvme((void *)vmeaddr, VME_LENGTH, verbose);
  fbase = vme_virt_addr + FLASHBASE;
  p_addr = (int *) fbase;
  select_range(vme_virt_addr, 0+RANGENUMBER, verbose);
  // read header

  fgets(c_input,MAXLEN,rbtFile);
  fgets(c_input,MAXLEN,rbtFile);
  // design name
  fgets(c_input,MAXLEN,rbtFile);
  //    printf("%s",c_input);
  // architecture name
  fgets(c_input,MAXLEN,rbtFile);
  //    printf("%s",c_input);
  // part
  fgets(c_input,MAXLEN,rbtFile);
  //    printf("%s",c_input);
  // date
  fgets(c_input,MAXLEN,rbtFile);
  //    printf("%s",c_input);
  // bits
  fgets(c_input,MAXLEN,rbtFile);
  //    printf("%s",c_input);

  l_count = 0;
  //    printf("addr = %x\n",p_addr);
  if (!quiet && !verbose) {
    printf("\n");
  }
  while(fgets(c_input, MAXLEN,rbtFile) != NULL) {
    len = strlen(c_input) -1;
    //      printf("%s",c_input);

    line++;
    if((line % 10000) == 0) {
      if (verbose) {
        //          printf("line %d ",line);
        //          if (verbose) printf("time: %f clocks\n", (int) time(NULL) - timestamp); //timestamp = (int) clock();
      } else {
        if (!quiet) {
          int i;
          printf("\r");
          for (i = 0; i < line / 10000; i++) {
            printf(">");
          }
          for (i = line / 10000; i < 38; i++) {
            printf("-");
          }
          fflush(stdout);
        }
      }
    }
    for(i=0; i<len/8; i++) {
      l_word = 0;
      for(j=0; j<8; j++) {
        //              printf("%c ",c_input[j+8*i]);
        if(c_input[(7-j)+8*i] == '1') {
          l_word += (1<<j);
        }
        bits++;
      }
      //          printf("word = %x\n",l_word);
      if (!simulate) {
        prog_flash((int)p_addr,l_word, verbose);
      }
      p_addr++;
      l_count++;
      if(l_count == RANGELIMIT) {
        //          range++;
        select_range(vme_virt_addr, 1+RANGENUMBER, verbose);
        p_addr = (int *) fbase;
        /*int loopCounter = 0;
          int val = 0;
          range++;
          if (verbose) printf("selecting next range \n");

          while ((val != range + 1 + 64) && (loopCounter < 10))
          {
          val = select_range(vme_virt_addr, range + 1, verbose);
          loopCounter++;
          if (val != range + 1 + 64)
          {
          read_array(fbase);
          closevme(verbose);
          vme_virt_addr = (unsigned long)openvme((void *)vmeaddr,vme_len, verbose);
          fbase = vme_virt_addr + FLASHBASE;
          p_addr = (int *) fbase;
          }
          }*/

        //select_range(vme_virt_addr,range, verbose);
        //vme_virt_addr = saveSelectRange(vme_virt_addr, (void *)vmeaddr,vme_len, range + 1, verbose);
        //p_addr = (int *)fbase;
      }
    }
  }
  //  if (!quiet && !verbose) printf("\n");
  //    if (verbose) printf("File %s loaded, %d bits, %d bytes\n",fileName,bits,l_count);
  read_array(fbase);
  closevme(verbose);
}

bool verifyFlashMemory(int moduleNumber, FILE *rbtFile, bool simulate, bool verbose)
{
  unsigned long vmeaddr, vme_virt_addr;
  volatile int *p_addr;
  char c_input[MAXLEN];
  int i, j;
  int len;
  int l_word;
  int fbase;
  int data;
  int bits = 0;
  int err = 0;
  int line = 0;
  int l_count = 0;
  //    printf("\nverifying flash memory\n");
  printlog("verifing flash memory", true, !quiet);
  vmeaddr = moduleNumber * MODULE_LENGTH  ;
  vme_virt_addr  = (unsigned long)openvme((void *)vmeaddr, VME_LENGTH, verbose);
  fbase = vme_virt_addr + FLASHBASE;
  p_addr = (int *) fbase;
  select_range(vme_virt_addr, 0+RANGENUMBER, verbose);
  rewind(rbtFile);

  fgets(c_input,MAXLEN,rbtFile);
  fgets(c_input,MAXLEN,rbtFile);
  // design name
  fgets(c_input,MAXLEN,rbtFile);
  printlog("file information:", true, verbose);
  printlog(c_input, true, verbose);
  //    printf("%s",c_input);
  // architecture name
  fgets(c_input,MAXLEN,rbtFile);
  printlog(c_input, true, verbose);
  //    printf("%s",c_input);
  // part
  fgets(c_input,MAXLEN,rbtFile);
  printlog(c_input, true, verbose);
  //    printf("%s",c_input);
  // date
  fgets(c_input,MAXLEN,rbtFile);
  printlog(c_input, true, verbose);
  //    printf("%s",c_input);
  // bits
  fgets(c_input,MAXLEN,rbtFile);
  printlog(c_input, true, verbose);
  //    printf("%s",c_input);

  l_count = 0;

  //    printf("addr = %x\n",p_addr);
  if (!quiet && !verbose) {
    printf("\n");
  }
  while(fgets(c_input, MAXLEN,rbtFile) != NULL) {
    len = strlen(c_input) -1;
    //      printf("%s",c_input);

    line++;
    if((line % 10000) == 0) {
      if (verbose) {
        //          printf("line %d ",line);
        //          if (verbose) printf("time: %f clocks\n", (int) time(NULL) - timestamp); //timestamp = (int) clock();
      } else {
        if (!quiet) {
          int i;
          printf("\r");
          for (i = 0; i < line / 10000; i++) {
            printf(">");
          }
          for (i = line / 10000; i < 38; i++) {
            printf("-");
          }
          fflush(stdout);
        }
      }
    }
    for(i=0; i<len/8; i++) {
      l_word = 0;
      for(j=0; j<8; j++) {
        //              printf("%c ",c_input[j+8*i]);
        if(c_input[(7-j)+8*i] == '1') {
          l_word += (1<<j);
        }
        bits++;
      }
      //          printf("word = %x\n",l_word);
      //            prog_flash((int)p_addr,l_word);
      data = ntohl(*p_addr) & 0xff;
      if(data != l_word) {
        //          printf("ERROR: offset %x, written %x read %x\n",l_count,
        //                 l_word,data);
        err++;
      }
      //      if(err > 100)goto err_exit;

      p_addr++;
      l_count++;
      if(l_count == RANGELIMIT) {
        //          range++;
        select_range(vme_virt_addr, 1, verbose);
        p_addr = (int *) fbase;
      }
    }
  }
  //  if (!quiet && !verbose) printf("\n");
  //
  //    printf("File %s verified, %d bits, %d bytes\n", fileName, bits, l_count);
  closevme(verbose);
  // err_exit:
  //     return_controller(vme_virt_addr,vme_len);
  //    exit(1);

  if (err == 0) {
    return true;
  } else {
    return false;
  }
}


main (int argc, char *argv[])
{
  char fileName[80];
  int moduleNumber;
  unsigned long vmeaddr;
  unsigned long vme_virt_addr;
  FILE *rbtFile;
  bool memoryVerified;

  bool eraseFlash = false;
  bool verifyFlash = false;
  bool restartModule =  false;
  bool displayHelp = false;
  bool writeFlash = false;
  bool verbose = false;
  bool simulate = false;

  int numberOptions = 0;
  int i;
  timestamp = (int) time(NULL);

  quiet = false;
  verbose = false;
  memoryVerified = false;

  //signalhandler
  signal(SIGKILL,(void *)sighandler);
  signal(SIGINT,(void *)sighandler);
  signal(SIGQUIT,(void *)sighandler);
  signal(SIGTERM,(void *)sighandler);
  signal(SIGBUS,(void *)sighandler);
  signal(SIGSEGV,(void *)sighandler);

  //extract options from command line
  for (i = 1; i < argc; i++) {
    if(strncmp(argv[i], "-",1) == 0) {
      numberOptions++;
      if(strncmp(argv[i], "--", 2) == 0) {
        if(strcmp(argv[i], "--erase") == 0) {
          eraseFlash = true;
        }
        if(strcmp(argv[i], "--write") == 0) {
          writeFlash = true;
        }
        if(strcmp(argv[i], "--verify") == 0) {
          verifyFlash = true;
        }
        if(strcmp(argv[i], "--restart") == 0) {
          restartModule = true;
        }
        if(strcmp(argv[i], "--help") == 0) {
          displayHelp = true;
        }
        if(strcmp(argv[i], "--simulate") == 0) {
          simulate = true;
        }
        if(strcmp(argv[i], "--verbose") == 0) {
          verbose = true;
          quiet = false;
        }
        if(strcmp(argv[i], "--quiet") == 0) {
          quiet = true;
          verbose = false;
        }
      } else {
        if(strchr(argv[i], 'e') != 0) {
          eraseFlash = true;
        }
        if(strchr(argv[i], 'w') != 0) {
          writeFlash = true;
        }
        if(strchr(argv[i], 'y') != 0) {
          verifyFlash = true;
        }
        if(strchr(argv[i], 'r') != 0) {
          restartModule = true;
        }
        if(strchr(argv[i], 's') != 0) {
          simulate = true;
        }
        if(strchr(argv[i], 'v') != 0) {
          verbose = true;
          quiet = false;
        }
        if(strchr(argv[i], 'q') != 0) {
          quiet = true;
          verbose = false;
        }
        if(strchr(argv[i], 'h') != 0) {
          displayHelp = true;
        }

      }
    }
  }

  //display Help
  if (displayHelp) {
    printf("usage: ProgramVUPROM [-ewyrsvq] moduleNumber [file]\n");
    printf("Program VUPROM module.\n\n");
    printf("   -e, --erase          erase flash memory\n   -w, --write          write new program to flash memory\n   -y, --verify         verify flash memory\n");
    printf("   -r, --restart        restart Module\n   -s, --simulate       don\'t make any changes to the VUPROMS\n");
    printf("   -v, --verbose        print a lot of information\n   -q, --quiet          do not print any information except for error messages\n");
    printf("   -h, --help           print this help\n\n");
    printf("");
    exit(0);
  }

  //try to open log file
  openLog(verbose);

  //check for right number of parameters and extract module number and filename form commandline
  printlog("parsing commandline ... ", true, verbose);
  if((writeFlash ||verifyFlash) && (argc - numberOptions == 3)) {
    sscanf(argv[1 + numberOptions],"%x",&moduleNumber);
    strcpy(fileName,argv[2 + numberOptions]);
    if (strspn(argv[1 + numberOptions], "0123456789abcdefABCDEF") != strlen(argv[1 + numberOptions])) {
      printf("module number is not given\n");
      printf("usage: ProgramVUPROM [-ewyrsvq] moduleNumber [file]\n");
      printlog("error parsing commandline, aborting", true, false);
      closeLog(verbose);
      exit(1);
    }
  } else {
    //rbt file is only needed when writing or veryfiing flash memory
    if(!(writeFlash ||verifyFlash) && (argc - numberOptions == 2)) {
      sscanf(argv[1 + numberOptions],"%x",&moduleNumber);
      if (strspn(argv[1 + numberOptions], "0123456789abcdefABCDEF") != strlen(argv[1 + numberOptions])) {
        printf("module number is not given\n");
        printf("usage: ProgramVUPROM [-ewyrsvq] moduleNumber [file]\n");
        printlog("error parsing commandline, aborting", true, false);
        closeLog(verbose);
        exit(1);
      }
    } else {
      printf("wrong number of command line arguments!\n");
      printf("usage: ProgramVUPROM [-ewyrsvq] moduleNumber [file]\n");
      printlog("error parsing commandline, aborting", true, false);
      closeLog(verbose);
      exit(1);
    }
  }
  printlog("done", false, verbose);

  //logging general information
  printlog("options are: ", true, verbose);
  if (eraseFlash) {
    printlog("eraseFlash ", false, verbose);
  }
  if (writeFlash) {
    printlog("writeFlash ", false, verbose);
  }
  if (verifyFlash) {
    printlog("verifyFlash ", false, verbose);
  }
  if (restartModule) {
    printlog("restartModule ", false, verbose);
  }
  if (simulate) {
    printlog("simulate ", false, verbose);
  }
  printlog("module number: ", true, verbose);
  printlogInt(moduleNumber, verbose);
  if (writeFlash || verifyFlash) {
    printlog("filename: ", true, verbose);
    printlog(fileName, false, verbose);
  }

  //open rbt file
  if (writeFlash) {
    printlog("opening rbt file ... ", true, verbose);
    if((rbtFile = fopen(fileName,"r")) == (FILE *)NULL) {
      perror ("fopen");
      printlog("error opening rbt file, aborting", true, verbose);
      closeLog(verbose);
      exit(1);
    }
    printlog("done", false, verbose);
  }

  // erase flash memory if requested
  if (eraseFlash) {
    eraseFlashMemory(moduleNumber, simulate, verbose);
  }

  //flash program on module if requested
  if (writeFlash) {
    writeFlashMemory(moduleNumber, rbtFile, simulate, verbose);
  }

  //verify program in flash memory if requested
  if (verifyFlash) {
    memoryVerified = verifyFlashMemory(moduleNumber, rbtFile, simulate, verbose);
  }

  //restart module if requested
  if (restartModule) {
    if (memoryVerified || !verifyFlash) {
      printlog("restarting module ... ", true, !quiet);
      vmeaddr = moduleNumber * MODULE_LENGTH  ;
      vme_virt_addr  = (unsigned long)openvme((void *)vmeaddr, VME_LENGTH, verbose);
      if (!simulate) {
        restart(vme_virt_addr, 0+RANGENUMBER);
      }
      closevme(verbose);
      printlog("done", false, !quiet);
    } else {
      printlog("not restarting module due to memory verification errors", true, true);
    }
  }

  if (!quiet) {
    printf("\n");
  }
  closeLog(verbose);
  exit(0);
}
