# If you are a Eciadsl project member, the best thing to do is :
# $ ssh -l yourlogin eciadsl.sf.net
# (when asked for a password, type Enter)
# $ cd /home/groups/e/ec/eciadsl/htdocs/doc
# $ umask 002 (to make file group writable)
# $ make site (to get the last Tex and Makefile using anonymous cvs)
# $ make (to get your doc in DVI HTML PS PDF ASCII)

# 08/02/2002: pdf generation has been removed since it does not work on
# 			  eciadsl.sf.net since the addition of \usepackage{html}

base = howto
pwd = `pwd`
cvsroot= :pserver:anonymous@cvs.eciadsl.sourceforge.net:/cvsroot/eciadsl

all: $(base).dvi $(base).html $(base).ps $(base).txt \
	kernelmode-design.html technical-reference.html \
	keep-useful-files-only

%.aux: %.tex
	latex $<

%.dvi: %.tex
# we run latex twice to get a proper table of content in the .dvi file
	$(RM) $*.aux
	latex $<
	latex $<

%.html: %.tex %.aux
	latex2html -dir $(pwd) -nonavigation -noauto_link -ascii_mode -show_section_numbers -split 0 $<
# we removed reference to howto.html
	sed s/$@// $@ > tmp.html
	mv tmp.html $@

%.ps: %.dvi 
	dvips -f $< > $@

%.pdf: %.tex
	$(RM) $*.aux
	pdflatex $<
	pdflatex $<

%.txt: %.html
	lynx -dump $< > $@

site:	
	cvs -d$(cvsroot) login
	cvs -d$(cvsroot) update -A -d
	cvs -d$(cvsroot) logout

keep-useful-files-only:
	$(RM) internals.pl labels.pl warnings *.log *.aux *.toc *.out

clean: keep-useful-files-only
	$(RM) *.html *.css *.dvi *.ps *.pdf *.txt
