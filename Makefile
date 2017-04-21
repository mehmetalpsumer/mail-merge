mm: mm.c
	gcc -o mm mm.c array.c -I /usr/include/libxml2 -lxml2
run:
	./mm -o mails -v variables.xml -t template.xml
