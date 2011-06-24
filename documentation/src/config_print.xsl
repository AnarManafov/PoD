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
    
    <!-- Custom font settings -->
    <xsl:param name="title.font.family">sans-serif,SimHei</xsl:param>
    <xsl:param name="body.font.family">serif,SimSun</xsl:param>
    <xsl:param name="sans.font.family">sans-serif,SimHei</xsl:param>
    <xsl:param name="dingbat.font.family">serif,SimSun</xsl:param>
    <xsl:param name="monospace.font.family">monospace,FangSong,SimSun</xsl:param>
    
    <!-- Do not put 'Chapter' at the start of eg 'Chapter 1. Doing This' -->
    <xsl:param name="local.l10n.xml" select="document('')"/>
    <l:i18n xmlns:l="http://docbook.sourceforge.net/xmlns/l10n/1.0">
        <l:l10n language="en">
            <l:context name="title-numbered">
                <l:template name="chapter" text="%n.&#160;%t"/>
            </l:context>
        </l:l10n>
    </l:i18n>
    
    <!-- border and shade to screen and programlisting -->
    <xsl:param name="shade.verbatim" select="1"/>
    <xsl:attribute-set name="shade.verbatim.style">
        <xsl:attribute name="background-color">#EFF5FB</xsl:attribute>
        <xsl:attribute name="border-width">0.5pt</xsl:attribute>
        <xsl:attribute name="border-style">solid</xsl:attribute>
        <xsl:attribute name="border-color">#DCDCDC</xsl:attribute>
        <xsl:attribute name="padding">3pt</xsl:attribute>
        <!--xsl:attribute name="margin">2pt</xsl:attribute-->
    </xsl:attribute-set>
    
    <!-- Customizing admonitions -->
    <xsl:template match="note|caution|warning|tip|important">
        <xsl:choose>
            <xsl:when test="$admon.graphics != 0">
                <fo:block border-style="solid" border-width="0.5pt"
                    border-color="#DCDCDC" background-color="#F7F2E0"
                    padding="3pt">
                    <xsl:call-template name="graphical.admonition"/>
                </fo:block>
            </xsl:when>
            <xsl:otherwise>
                <xsl:call-template name="nongraphical.admonition"/>
            </xsl:otherwise>
	    </xsl:choose>
    </xsl:template>
    
    <!-- Tables -->
    <!-- Some padding inside tables -->
    <xsl:attribute-set name="table.cell.padding">
        <xsl:attribute name="padding-left">3pt</xsl:attribute>
        <xsl:attribute name="padding-right">3pt</xsl:attribute>
        <xsl:attribute name="padding-top">3pt</xsl:attribute>
        <xsl:attribute name="padding-bottom">3pt</xsl:attribute>
    </xsl:attribute-set>
    
    <!-- Style tables -->
    <xsl:param name="table.cell.border.color">#D3D2D1</xsl:param>
    <xsl:param name="table.frame.border.color">#D3D2D1</xsl:param>
    <xsl:param name="table.cell.border.thickness">0.8pt</xsl:param>
    
    <xsl:param name="table.frame.border.thickness">0.8pt</xsl:param>
    <xsl:param name="table.cell.border.right.color">white</xsl:param>
    <xsl:param name="table.cell.border.left.color">#D3D2D1</xsl:param>
    <xsl:param name="table.frame.border.right.color">white</xsl:param>
    <xsl:param name="table.frame.border.left.color">white</xsl:param>
    
    <xsl:template name="table.cell.block.properties">
        <!-- highlight this entry? -->
        <xsl:if test="ancestor::thead or ancestor::tfoot">
            <xsl:attribute name="text-align">center</xsl:attribute>
            <xsl:attribute name="font-weight">bold</xsl:attribute>
            <xsl:attribute name="background-color">#DCDCDC</xsl:attribute>
            <xsl:attribute name="color">black</xsl:attribute>
        </xsl:if>
    </xsl:template>
    
    
</xsl:stylesheet>
