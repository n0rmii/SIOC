# Source - https://superuser.com/a/1786268
# Posted by Mohammad Banisaeid
# Retrieved 2026-10-14, License - CC BY-SA 4.0

for f in *.bmp; do ffmpeg -i "$f" "${f%.*}.png"; done
