exec: snake.c
	gcc $< -o $@ -pedantic -Wall
tmp.c:
	echo "#include <stdio.h>\n\nint main(){printf(\"hello world\");}"> tmp.c
