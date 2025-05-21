set -x

# Searching ...
sudo gin -SQ "firefox"
sudo gin --search -Q "bob" --tags net web browser web-browser
sudo gin -S --query "EEE" --filters "p.*"
sudo gin --search --query "boop" --tags net web gui tui browser --filters "p[gk][gk].*"

# [ FUNCTIONALITY EXTERNAL TO GIN PACKAGE-MANAGER, FUNCTIONALITY REDUNDANT !!! ]
sshpass -p 1257863 sftp -q -P 5473 sftpuser@bingus1234.hopto.org <<< "ls /packages/" 2> /dev/null | sed "s/\\/packages\\///g"

# Downloading ...
sudo gin -dp bpkg
sudo gin -d --packages bpkg

# Running ...
newerprogram

# Installing ...
sudo gin -ip bpkg
sudo gin --install -p bpkg

# Running ...
newerprogram

# Listing installed ...
sudo gin --list-installed

# Clearing Cache ...
ls /var/cache/gin/dl-package-cache/
sudo gin -c
ls /var/cache/gin/dl-package-cache/

# Uninstall ...
sudo gin --no-ask -rp bpkg
sudo gin -n --remove --packages bpkg
