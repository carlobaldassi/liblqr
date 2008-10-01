name = liblqr_manual
source = $(name).docbook
dest = html/index.html
dest_op = $(name)_onepage.html
main = $(name).html

stylesheet = lqr_style.xsl
stylesheet_op = lqr_style_onepage.xsl
redir = redir-html

.PHONY : all sgml xml onepage

all: html

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

onepage: $(dest_op)
	
$(dest_op): $(source) $(stylesheet_op)
	xsltproc -o $(dest_op) $(stylesheet_op) $(source)


clean:
	rm -f html/* $(main) $(dest_op)
