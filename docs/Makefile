name = liblqr_manual
source = $(name).docbook
dest = html/index.html
dest_op = $(name)_onepage.html
dest_man = man3/lqr_carver_new.3
dest_dist_man = ../man/lqr_carver_new.3
main = $(name).html

stylesheet = lqr_style.xsl
stylesheet_op = lqr_style_onepage.xsl
stylesheet_man = lqr_style_man.xsl
redir = redir-html

.PHONY : all sgml xml onepage man dist-man clean-dist-man

all: html

dist: html onepage dist-man

#sgml : $(dest)

#$(dest): $(source)
#	docbook2html $(source) -o html

html: $(main)

$(main) : $(dest)
	cp -f $(redir) $(main)

$(dest): $(source) $(stylesheet)
	[ -d html ] || mkdir html
	cd html; \
	xsltproc ../$(stylesheet) ../$(source)

man : $(dest_man)

$(dest_man) : $(source) $(stylesheet_man)
	[ -d man3 ] || mkdir man3
	xsltproc -o man3/ $(stylesheet_man) $(source)

onepage: $(dest_op)
	
$(dest_op): $(source) $(stylesheet_op)
	xsltproc -o $(dest_op) $(stylesheet_op) $(source)

dist-man : $(dest_dist_man)

$(dest_dist_man) : man
	[ -d ../man ] || mkdir ../man; \
	rm ../man/*; \
	cp man3/*.3 ../man; \
	./gen-man-makefile.am

clean:
	rm -f html/* man3/* $(main) $(dest_op)

clean-dist-man:
	rm -f ../man/*.3
