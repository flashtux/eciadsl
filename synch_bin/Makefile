create_md5sum: 
	for FILE in *.bin; \
	do \
		md5sum -b $$FILE > $$FILE.md5; \
	done

check_md5sum:
	for FILE in *.bin; \
	do \
		md5sum -c $$FILE.md5; \
	done

clear_md5sum:
	$(RM) *.md5
