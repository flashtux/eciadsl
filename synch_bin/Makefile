default:
	@echo "no default rule, use force_create_md5sum, create_md5sum, check_md5sum or clear_md5sum"

force_create_md5sum: 
	@for FILE in *.bin; \
	do \
		md5sum -b $$FILE > $$FILE.md5; \
	done

create_md5sum: 
	@for FILE in *.bin; \
	do \
		if [ ! -f $$FILE.md5 ]; \
		then \
			md5sum -b $$FILE > $$FILE.md5; \
		fi; \
	done

check_md5sum:
	@for FILE in *.bin; \
	do \
		if [ -f $$FILE.md5 ]; \
		then \
			md5sum -c $$FILE.md5; \
		fi; \
	done

clear_md5sum:
	$(RM) *.md5
