i=1
for file in elvis*; do
	mv "$file" "elvis$i.jpg"
	((i++))
done
