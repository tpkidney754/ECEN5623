#!/bin/bash
echo "ubuntu:Eagle2016" | chpasswd;
while IFS='' read -r email || [[ -n "$email" ]]; do
  user=${email/"@my.erau.edu"/""};
  user=${user/","/""};
  echo $(printf "%s -> %s" "$email" "$user");
  deluser --remove-home $user;
  adduser --shell /bin/bash --disabled-password --gecos "" $user;
  echo "$user:$(printf "jet%s" "$user")" | chpasswd;
  usermod -a -G "sudo" "$user";
done < "$1"
