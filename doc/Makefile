# If you are a Eciadsl project member, the best thing to do is :
# $ ssh -l yourlogin eciadsl.sf.net
# (when asked for a password, type Enter)
# $ cd /home/groups/e/ec/eciadsl/doc
# $ make site (to get the last Tex and Makefile using anonymous cvs)
# $ make (to get your doc in DVI HTML PS PDF ASCII)

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

pdf: $(base).tex
	pdflatex $(base).tex

ascii: html
	lynx -dump $(base).html >$(base).txt
	
site:	
	cvs -d$(cvsroot) login
	cvs -d$(cvsroot) update $(base).tex
	cvs -d$(cvsroot) update Makefile
	cvs -d$(cvsroot) logout

keep-useful-files-only:
	rm -f labels.pl warnings $(base).log $(base).aux $(base).toc
	
clean: keep-useful-files-only
	rm -f $(base).html $(base).css $(base).dvi $(base).ps $(base).pdf $(base).txt
