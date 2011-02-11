
SRC=\
	PoD.xml\
	Introduction.xml\
	Requirements.xml\
	Installation.xml\
	Configuration.xml\
	Download.xml\
	How_to_run.xml\
	gLite_plugin.xml\
	LSF_plugin.xml\
        PBS_plugin.xml\
        SSH_plugin.xml\
        GridEngine_plugin.xml\
        Condor_plugin.xml\
	How_to_test.xml\
	Known_Issues.xml\
	Support.xml\
        Cmd_line_interface.xml\
        QuickStart.xml


DOCBOOK=PoD.xml

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
        PAConsole_OGE_plugin_snapshot.png\
	PAConsole_Preferences.png\
	PAConsole_LogInfo.png

IMG_DIR = img

HTML_OUT= \
        PoD.html\
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
        PBS_plugin.html\
        SSH_plugin.html\
        GridEngine_plugin.html\
        Condor_plugin.html

CSS_FILES=docbook.css

# stylesheet processor
XSLTPROC       = xsltproc
XSLTPROC_FLAGS = --nonet --xinclude

# FOP (for pdf)
FOP=fop
POD_PDF=PoD.pdf

# file to log print output to
LOGFILE = print.log


all: gen_html gen_pdf

$(HTML_OUT_DIR):
	@echo "Creating output directory for html doc [$(HTML_OUT_DIR)]..."
	@mkdir $(HTML_OUT_DIR)

$(PRINT_OUT_DIR):
	@echo "Creating output directory for pdf,ps docs [$(PRINT_OUT_DIR)]..."
	@mkdir $(PRINT_OUT_DIR)

$(HTML_OUT_DIR)/%.css:
	@echo "Using custom CSS..."
	@cp $(CSS_FILES) $(HTML_OUT_DIR)/

# chunked html, on a chapter-by-chapter basis
html-docs: $(SRC)
	@echo "Generating html doc..."
	$(XSLTPROC) $(XSLTPROC_FLAGS) -o $(HTML_OUT_DIR)/ config.xsl $(XSL_HTML_CHUNK_STYLE) $(DOCBOOK)

# Print formats
# pdf and postscript
#FOP
#http://www.sagehill.net/docbookxsl/InstallingAnFO.html
#needed a hyphenation pattern http://offo.sourceforge.net/
print-docs:
	@echo "Generating PDF file: $(HTML_OUT_DIR)/$(POD_PDF) (please be patient)...";
	$(XSLTPROC) $(XSLTPROC_FLAGS) -o $(HTML_OUT_DIR)/index.fo config_print.xsl $(DOCBOOK) && \
        (cd $(HTML_OUT_DIR) && \
         $(FOP) index.fo $(POD_PDF) &> $(LOGFILE) < /dev/null && \
         echo "Generating PS file: $(HTML_OUT_DIR)/PoD.ps ..." && \
         pdftops $(POD_PDF) && \
         rm -f *.log *.aux *.fo *.out)


$(HTML_OUT_DIR)/%.png:
	@echo "Copying images..."
	@cp ${addprefix $(IMG_DIR)/, $(IMGS)} $(HTML_OUT_DIR)/

css_files: ${addprefix $(HTML_OUT_DIR)/, $(CSS_FILES)}
img_files: ${addprefix $(HTML_OUT_DIR)/, $(IMGS)}

gen_html: $(HTML_OUT_DIR) css_files img_files html-docs

gen_pdf:  $(HTML_OUT_DIR) img_files print-docs

clean:
	@-rm -rf $(HTML_OUT_DIR)
