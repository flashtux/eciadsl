# Just do "make" and you'll get your doc in the following formats
# dvi html ps pdf ascii

base = howto
pwd = `pwd`

all: dvi html ps pdf ascii keep-useful-files-only

dvi: $(base).tex
	latex $(base).tex

html: dvi 
	latex2html -dir $(pwd) -nonavigation -noauto_link -ascii_mode -show_section_numbers -split 0 howto howto.tex	
	
ps: dvi 
	dvips -f $(base).dvi >$(base).ps

pdf: dvi
	dvipdf $(base).dvi

ascii: html
	lynx -dump $(base).html >$(base).txt

site:	clean site2
site2:
	export CVSROOT=:ext:rougier@cvs.eciadsl.sourceforge.net:/cvsroot/eciadsl
	export CVS_RSH=ssh
	cvs update $(base).tex	
	make

keep-useful-files-only:
	rm -f labels.pl warnings $(base).log $(base).aux $(base).toc
	
clean: keep-useful-files-only
	rm -f $(base).html $(base).css $(base).dvi $(base).ps $(base).pdf $(base).txt
