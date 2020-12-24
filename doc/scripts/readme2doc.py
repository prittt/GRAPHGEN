import markdown as md

header = f"""
/**
@mainpage Documentation

<!-- <div style="width: 85%; margin: 0 auto;"> -->
<img src="graphgen_inline_text.png" height=110px style="display: block; margin-left: auto;  margin-right: auto;"/>
<br>
<!-- <div style="width: 70%;"> -->
<h2 class="num">Introduction</h2>
"""

footer = f"""
<!--
</div>
</div>
-->
*/
"""

with open("README.md", "r") as readme:
    data = readme.read()

data = data.split("<!--MAIN-DATA-BEGIN-->")[1]
#data = data.replace("\r\n", "")
#data = data.replace("\n", "")
html = md.markdown(data, extensions=['fenced_code'])

html = html.replace("<h2>", "<h2 class='num'>")
html = html.replace("<h3>", "<h3 class='num'>")
html = html.replace("<h4>", "<h4 class='num'>")
#html = html.replace(">\r\n", ">")
#html = html.replace(">\n", ">")

# ref_pos = html.find("<!--REFERENCES-BEGIN-->")

html = html.replace('<p align="justify">', '')
html = html.replace('</p>', '')

# Improve format of References
# html = html.replace('style="vertical-align: top !important;"', '')


# Improve format of Contributors
html = html.replace("<sub><b>", "<sub><br/><b>")

file_path = "doc/doxygen/mainpages/home.h"
with open(file_path, "w+") as homefile:
    homefile.write(header)
    homefile.write(html)
    homefile.write(footer)
