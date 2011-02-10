<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:fo="http://www.w3.org/1999/XSL/Format"
                version="1.0">

<xsl:import href="http://docbook.sourceforge.net/release/xsl/current/html/docbook.xsl"/>
<xsl:import href="http://docbook.sourceforge.net/release/xsl/current/html/chunk-common.xsl"/>
<xsl:import href="http://docbook.sourceforge.net/release/xsl/current/html/manifest.xsl"/>
<xsl:import href="http://docbook.sourceforge.net/release/xsl/current/html/chunk-code.xsl"/>

<!-- use 8859-1 encoding -->
<xsl:output method="html" encoding="ISO-8859-1" indent="yes"/>


<!-- ignor image scaling in HTML -->
	<xsl:param name="ignore.image.scaling" select="'1'"/>
        
	<xsl:param name="use.id.as.filename" select="'1'"/>
	<xsl:param name="admon.graphics" select="'1'"/>
	<xsl:param name="admon.graphics.path"></xsl:param>
	<xsl:param name="chunk.section.depth" select="0"></xsl:param>
	<xsl:param name="generate.section.toc.level" select="1"></xsl:param>
	<xsl:param name="section.autolabel" select="1"></xsl:param>
	<xsl:param name="section.autolabel.max.depth" select="1"></xsl:param>
	<xsl:param name="section.label.includes.component.label" select="1"></xsl:param>
	<xsl:param name="html.stylesheet" select="'docbook.css'"/>
	<xsl:template name="user.header.content">
		<div class="projection">
			<div class="logo">
				<img src="PoD_logo.png" height="122" widht="250" style="position: top: 1em; left;"/>
			</div>
		</div>
	</xsl:template>
</xsl:stylesheet>

