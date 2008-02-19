<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:fo="http://www.w3.org/1999/XSL/Format"
                version="1.0">
 
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
				<img src="http://www-linux.gsi.de/~manafov/D-Grid/Trac_Logo.png" 
  						style="position: top: 1em; left;"/>
  			</div>
		</div>
	</xsl:template>
</xsl:stylesheet>
