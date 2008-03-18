
SRC=\
	gLitePROOF.docbook\
	Introduction.docbook\
	Requirements.docbook\
	Installation.docbook\
	Configuration.docbook\
	Download.docbook\
	How_to_run.docbook\
	How_to_test.docbook\
	Known_Issues.docbook\
	Support.docbook


DOCBOOK=gLitePROOF.docbook

HTML_OUT_DIR=../html_help

IMGS= \
	gLitePROOF_logo.png\
	gLite_PROOF.png\
	important.png\
	tip.png\
	warning.png\
	note.png\
	PAConsole_ServersOff.png\
	PAConsole_ServersOn.png\
	PAConsole_Grid0.png\
	PAConsole_Grid1.png\
	PAConsole_Grid2.png\
	PAConsole_Workers.png

IMG_DIR = img

HTML_OUT= \
	Configuration.html\
	How_to_run.html\
	Introduction.html\
	Support.html\
	index.html\
	Known_Issues.html\
	How_to_test.html\
	download.html\
	Installation.html\
	Requirements.html

CSS_FILES=docbook.css

XMLTO=xmlto

XMLTO_PARAM=-o $(HTML_OUT_DIR) xhtml -m config.xsl
XMLTO_PARAM_NOCHUNKS=-o $(HTML_OUT_DIR) xhtml-nochunks -m config.xsl

all: gen_html

$(HTML_OUT_DIR):
	@echo "Creating output directory for html doc [$(HTML_OUT_DIR)]..."
	@mkdir $(HTML_OUT_DIR)

$(HTML_OUT_DIR)/%.css:
	@echo "Using custom CSS..."
	@cp $(CSS_FILES) $(HTML_OUT_DIR)/

$(HTML_OUT_DIR)/%.html: $(SRC)
	@echo "Generating html doc..."
	$(XMLTO) $(XMLTO_PARAM) $(DOCBOOK)
	$(XMLTO) $(XMLTO_PARAM_NOCHUNKS) $(DOCBOOK)

$(HTML_OUT_DIR)/%.png:
	@echo "Copying images..."
	@cp ${addprefix $(IMG_DIR)/, $(IMGS)} $(HTML_OUT_DIR)/

html_files: ${addprefix $(HTML_OUT_DIR)/, $(HTML_OUT)}
css_files: ${addprefix $(HTML_OUT_DIR)/, $(CSS_FILES)}
img_files: ${addprefix $(HTML_OUT_DIR)/, $(IMGS)}

gen_html: $(HTML_OUT_DIR) css_files img_files html_files


clean:
	@-rm -rf $(HTML_OUT_DIR)
