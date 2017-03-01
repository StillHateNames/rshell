#!/bin/sh
../rshell
< "echo \"Hello testers!\"\|\|ls -a\|\|ls\|\|\#prank\|\|xD\n"
< "ls -a\&\&echo \"Hello tes\#ters!\"\&\&ls\&\&prank\&\&xD\n"
< "ls\;echo \"Hello testers!\"\;ls -a\;ls\;pra\#nk\;xD\n"
< "prank\;echo \"Hell\#o testers!\"\&\&ls -a\&\&ls\|\|prank\;xD\n"
< "xD\&\#\|echo \"Hello testers!\"\|\|ls -a\|\|ls\|\|prank\|\|xD\n"
< "echo \"Hello \#testers!\"\n"
< "ls \#-a\n"
< "ls\n"
< "pr\#ank\n"
< "\#xD\n"
