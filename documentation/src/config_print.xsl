<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:fo="http://www.w3.org/1999/XSL/Format"
                version="1.0">

<xsl:import href="http://docbook.sourceforge.net/release/xsl/current/fo/docbook.xsl"/>

<!-- set indent = yes while debugging, then change to NO -->
<xsl:output method="xml" indent="no"/>


     	<xsl:param name="use.id.as.filename" select="'1'"/>
	<xsl:param name="admon.graphics" select="'1'"/>
	<xsl:param name="admon.graphics.path"></xsl:param>
	<xsl:param name="chunk.section.depth" select="0"></xsl:param>
	<xsl:param name="generate.section.toc.level" select="1"></xsl:param>
	<xsl:param name="section.autolabel" select="1"></xsl:param>
	<xsl:param name="section.autolabel.max.depth" select="1"></xsl:param>
	<xsl:param name="section.label.includes.component.label" select="1"></xsl:param>
	<xsl:param name="html.stylesheet" select="'docbook.css'"/>
	<!--xsl:template name="user.header.content">
		<div class="projection">
			<div class="logo">
				<img src="PoD_logo.png" height="122" widht="250" style="position: top: 1em; left;"/>
			</div>
		</div>
	</xsl:template-->



<xsl:param name="paper.type">A4</xsl:param>
<!-- scale images in FO -->
<xsl:param name="ignore.image.scaling" select="'0'"/>
<!-- don't indent -->
<xsl:param name="body.start.indent" select="'0pt'"/>
<!-- output in 'block' mode -->
<xsl:param name="variablelist.as.blocks" select="1"/>
<!-- don't show url separately in ulinks -->
<xsl:param name="ulink.show" select="0"/>
<!-- show links in color -->
<xsl:attribute-set name="xref.properties">
  <xsl:attribute name="color">blue</xsl:attribute>
</xsl:attribute-set>

<!-- Do not put 'Chapter' at the start of eg 'Chapter 1. Doing This' -->
<xsl:param name="local.l10n.xml" select="document('')"/>
<l:i18n xmlns:l="http://docbook.sourceforge.net/xmlns/l10n/1.0">
  <l:l10n language="en">
    <l:context name="title-numbered">
      <l:template name="chapter" text="%n.&#160;%t"/>
    </l:context>
  </l:l10n>
</l:i18n>

<!-- colored background for programlisting and screen -->
<!-- setting param shade.verbatim=1 screws up literallayout -->
<!-- something chronic, so have to go this route -->
<xsl:template match="programlisting|screen|synopsis">
  <xsl:param name="suppress-numbers" select="'0'"/>
  <xsl:variable name="id"><xsl:call-template name="object.id"/></xsl:variable>
  <xsl:variable name="content">
    <xsl:choose>
      <xsl:when test="$suppress-numbers = '0'
                      and @linenumbering = 'numbered'
                      and $use.extensions != '0'
                      and $linenumbering.extension != '0'">
        <xsl:call-template name="number.rtf.lines">
          <xsl:with-param name="rtf">
            <xsl:apply-templates/>
          </xsl:with-param>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <fo:block id="{$id}" white-space-collapse='false' white-space-treatment='preserve'
            linefeed-treatment="preserve"  background-color="#f2f2f9"
            xsl:use-attribute-sets="monospace.verbatim.properties">
    <xsl:choose>
      <xsl:when test="$hyphenate.verbatim != 0
                      and function-available('exsl:node-set')">
        <xsl:apply-templates select="exsl:node-set($content)"
                             mode="hyphenate.verbatim"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:copy-of select="$content"/>
      </xsl:otherwise>
    </xsl:choose>
  </fo:block>
</xsl:template>



</xsl:stylesheet>

