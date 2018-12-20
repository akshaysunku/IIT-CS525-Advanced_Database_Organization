
default: test_1

test_1: test_case_1.o buffer_mgr.o storage_mgr.o dberror.o buffer_mgr_stat.o
	gcc -o test_1 test_case_1.o buffer_mgr.o storage_mgr.o dberror.o buffer_mgr_stat.o


test_case_1.o: test_assign2_1.c storage_mgr.h dberror.h test_helper.h buffer_mgr_stat.h
	gcc -o test_case_1.o -c test_assign2_1.c -lm


buffer_mgr.o: buffer_mgr.c storage_mgr.h dberror.h test_helper.h buffer_mgr_stat.h
		gcc -c buffer_mgr.c -o buffer_mgr.o

storage_mgr.o: storage_mgr.c storage_mgr.h dberror.h test_helper.h buffer_mgr_stat.h
		gcc -c storage_mgr.c -o storage_mgr.o

dberror.o: dberror.c dberror.h
		gcc -c dberror.c

buffer_mgr_stat.o: buffer_mgr_stat.c storage_mgr.h dberror.h test_helper.h buffer_mgr_stat.h
		gcc -c buffer_mgr_stat.c

.PHONY: clean
clean:
	$(RM) test_1 *.o *~

run:
	./test_1


