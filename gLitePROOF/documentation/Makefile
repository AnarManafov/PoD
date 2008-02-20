
DOCBOOK=gLitePROOF.docbook

HTML_OUT_DIR=html

HTML_OUT= \
	Configuration.html\
	How_to_run.html\
	Introduction.html\
	Support.html\
	index.html\
	Known_Issues.html\
	Test.html\
	download.html\
	Installation.html\
	Requirements.html

CSS_FILES=docbook.css

XMLTO=xmlto

XMLTO_PARAM=-o $(HTML_OUT_DIR) xhtml -m config.xsl

all: gen_html

$(HTML_OUT_DIR):
	@echo "Creating output directory for html doc [$(HTML_OUT_DIR)]..."
	@mkdir $(HTML_OUT_DIR)

$(HTML_OUT_DIR)/%.css:
	@echo "Using custom CSS..."
	@cp $(CSS_FILES) $(HTML_OUT_DIR)/

$(HTML_OUT_DIR)/%.html:
	@echo "Generating html doc..."
	$(XMLTO) $(XMLTO_PARAM) $(DOCBOOK)

html_files: ${addprefix $(HTML_OUT_DIR)/, $(HTML_OUT)}
css_files:  ${addprefix $(HTML_OUT_DIR)/, $(CSS_FILES)}

gen_html: $(HTML_OUT_DIR) css_files html_files


clean:
	@-rm -rf $(HTML_OUT_DIR)
