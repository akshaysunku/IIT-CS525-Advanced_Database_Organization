

default: test_1

test_1: test_case_1.o buffer_mgr.o storage_mgr.o dberror.o buffer_mgr_stat.o expr.o record_mgr.o rm_serializer.o
	gcc -o test_1 test_case_1.o buffer_mgr.o storage_mgr.o dberror.o buffer_mgr_stat.o expr.o record_mgr.o rm_serializer.o

test_case_1.o: test_assign3_1.c storage_mgr.h dberror.h test_helper.h buffer_mgr_stat.h tables.h record_mgr.h
	gcc -o test_case_1.o -c test_assign3_1.c -lm

storage_mgr.o: storage_mgr.c storage_mgr.h dberror.h test_helper.h buffer_mgr_stat.h tables.h record_mgr.h
		gcc -c storage_mgr.c -o storage_mgr.o

buffer_mgr.o: buffer_mgr.c storage_mgr.h dberror.h test_helper.h buffer_mgr_stat.h tables.h record_mgr.h
		gcc -c buffer_mgr.c -o buffer_mgr.o

buffer_mgr_stat.o: buffer_mgr_stat.c storage_mgr.h dberror.h test_helper.h buffer_mgr_stat.h tables.h record_mgr.h
		gcc -c buffer_mgr_stat.c

dberror.o: dberror.c dberror.h
		gcc -c dberror.c

record_mgr.o: record_mgr.c storage_mgr.h dberror.h test_helper.h buffer_mgr_stat.h tables.h record_mgr.h
		gcc -c record_mgr.c -o record_mgr.o

expr.o: expr.c storage_mgr.h dberror.h test_helper.h buffer_mgr_stat.h tables.h record_mgr.h
		gcc -c expr.c -o expr.o

rm_serializer.o: rm_serializer.c storage_mgr.h dberror.h test_helper.h buffer_mgr_stat.h tables.h record_mgr.h
		gcc -c rm_serializer.c -o rm_serializer.o

.PHONY: clean
clean:
	$(RM) test_1 test_2 *.o *~

run:
	./test_1
