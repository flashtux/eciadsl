# For the users of the driver :
# Just do "make" and you'll get your doc in the following formats
# dvi html ps pdf ascii

# For the developers :
# Just do "make site" and the following actions will be auto executed :
# - gets the last Tex file via anonymous cvs
# - gets the last doc Makefile and move it to Makefile2
# - executes Makefile2
# - saves Makefile2 as Makefile

base = howto
pwd = `pwd`
cvsroot= :pserver:anonymous@cvs.eciadsl.sourceforge.net:/cvsroot/eciadsl

all: dvi html ps pdf ascii keep-useful-files-only

dvi: $(base).tex
	latex $(base).tex

html: dvi 
	latex2html -dir $(pwd) -nonavigation -noauto_link -ascii_mode -show_section_numbers -split 0 howto howto.tex	
	
ps: dvi 
	dvips -f $(base).dvi >$(base).ps

pdf: dvi
	dvips -j0 -P generic -q -f $(base).dvi | gs -q -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sOutputFile=$(base).pdf -dCompatibilityLevel=1.3 -c save pop -

ascii: html
	lynx -dump $(base).html >$(base).txt
	
site:	clean site2
site2:
	cvs -d$(cvsroot) login
	cvs -d$(cvsroot) update $(base).tex
	cvs -d$(cvsroot) update Makefile
	cvs -d$(cvsroot) logout
	mv Makefile Makefile2
	make -f Makefile2
	mv Makefile2 Makefile

keep-useful-files-only:
	rm -f labels.pl warnings $(base).log $(base).aux $(base).toc
	
clean: keep-useful-files-only
	rm -f $(base).html $(base).css $(base).dvi $(base).ps $(base).pdf $(base).txt
