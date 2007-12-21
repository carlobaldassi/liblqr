name = liblqr_manual
source = $(name).docbook
dest = html/index.html
main = $(name).html

stylesheet = lqr_style.xsl
redir = redir-html

.PHONY : all sgml xml


all: xml

#sgml : $(dest)

#$(dest): $(source)
#	docbook2html $(source) -o html

xml: $(main)

$(main) : $(dest)
	cp -f $(redir) $(main)

$(dest): $(source) $(stylesheet)
	[ -d html ] || mkdir html
	cd html; \
	xsltproc ../$(stylesheet) ../$(source)

clean:
	rm -f html/* $(main)
