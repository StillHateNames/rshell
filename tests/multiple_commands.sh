#!/bin/sh
../rshell
< "echo \"Hello testers!\"\|\|ls -a\|\|ls\|\|prank\|\|xD\n"
< "ls -a\&\&echo \"Hello testers!\"\&\&ls\&\&prank\&\&xD\n"
< "ls\;echo \"Hello testers!\"\;ls -a\;ls\;prank\;xD\n"
< "prank\;echo \"Hello testers!\"\&\&ls -a\&\&ls\|\|prank\;xD\n"
< "xD\&\|echo \"Hello testers!\"\|\|ls -a\|\|ls\|\|prank\|\|xD\n"
