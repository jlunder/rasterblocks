for file in ./test/tracks/*.mp3
do
	avconv -i "$file" "${file}".wav
done