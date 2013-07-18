#!/bin/sh
echo -n `vmeext $1 $2 $3 | cut -f 6 -d " " `
