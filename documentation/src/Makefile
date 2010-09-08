
SRC=\
	POD.docbook\
	Introduction.docbook\
	Requirements.docbook\
	Installation.docbook\
	Configuration.docbook\
	Download.docbook\
	How_to_run.docbook\
	gLite_plugin.docbook\
	LSF_plugin.docbook\
        PBS_plugin.docbook\
        SSH_plugin.docbook\
	How_to_test.docbook\
	Known_Issues.docbook\
	Support.docbook\
  Cmd_line_interface.docbook


DOCBOOK=POD.docbook

HTML_OUT_DIR=../html_help

IMGS= \
	PoD_logo.png\
	PoD_Components.png\
	important.png\
	tip.png\
	warning.png\
	note.png\
	PAConsole_Server_snapshot.png\
	PAConsole_Grid0.png\
	PAConsole_Grid1.png\
	PAConsole_Grid2.png\
	PAConsole_Grid3.png\
	PAConsole_LSF_plugin_snapshot.png\
	PAConsole_Workers_LSF_snapshot.png\
  PAConsole_PBS_plugin_snapshot.png\
	PAConsole_Preferences.png\
	PAConsole_LogInfo.png

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
	Requirements.html\
  Cmd_line_interface.html\
  LSF_plugin.html\
  PBS_plugin.html

CSS_FILES=docbook.css

XMLTO=xmlto

XMLTO_PARAM=-vv -o $(HTML_OUT_DIR) xhtml -m config.xsl
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
