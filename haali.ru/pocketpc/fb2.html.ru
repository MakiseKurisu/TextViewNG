<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
    "http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd">

<html>
  <head>
    <title>FictionBook 2.0</title>
    <link rel="stylesheet" type="text/css" href="=.css">
  </head>
  <body>
    <div class="topmenu">
      <div class="logo"><img class="noborder" src="logo.png"></div>
      <!--<div class="mirror">
	<a class="menu small" href="http://haali.rt.ru/pocketpc/">§©§Ö§â§Ü§Ñ§Ý§à 1</a><br>
	<a class="menu small" href="http://haali.cs.msu.ru/pocketpc/">§©§Ö§â§Ü§Ñ§Ý§à 2</a>
      </div>-->
      <a class="menu sep" href="index.html.ru">§¤§Ý§Ñ§Ó§ß§Ñ§ñ</a>
      <a class="menu sep" href="news.html.ru">§¯§à§Ó§à§ã§ä§Ú</a>
      <a class="menu sep" href="download.html.ru">§³§Ü§Ñ§é§Ñ§ä§î</a>
      <a class="menu sep" href="screenshots.html.ru">§¬§Ñ§â§ä§Ú§ß§Ü§Ú</a>
      <a class="menu sep" href="docs.html.ru">§¥§à§Ü§å§Þ§Ö§ß§ä§Ñ§è§Ú§ñ</a>
      <a class="menu sep" href="bugs.html.ru">§°§ê§Ú§Ò§Ü§Ú</a>
      <a class="menu sep" href="scripts.html.ru">FB Tools</a>
      <a class="menu sep" href="fb2.html.ru">FB2.0</a>
      <a class="menu" href="index.html.en">English page</a>
    </div>
    <div class="main">

<!-- vim: set syn=html : -->


<h3>FB 2.0</h3>
FB 2.0 - §Ù§ß§Ñ§é§Ú§ä§Ö§Ý§î§ß§à§Ö §á§â§à§Õ§Ó§Ú§Ø§Ö§ß§Ú§Ö §Ó§á§Ö§â§Ö§Õ §á§à §ã§â§Ñ§Ó§ß§Ö§ß§Ú§ð §ã §á§â§Ö§Õ§í§Õ§å§ë§Ú§Þ§Ú §Ó§Ö§â§ã§Ú§ñ§Þ§Ú.
§³§â§Ö§Õ§Ú §à§ã§ß§à§Ó§ß§í§ç §Ú§Ù§Þ§Ö§ß§Ö§ß§Ú§Û:<br>
* §µ§á§â§à§ã§ä§Ú§Ý§Ñ§ã§î §ã§ä§â§å§Ü§ä§å§â§Ñ §Õ§à§Ü§å§Þ§Ö§ß§ä§Ñ - §Ò§à§Ý§î§ê§Ö §ß§Ú§Ô§Õ§Ö §ß§Ö§ä §Ñ§ä§â§Ú§Ò§å§ä§Ñ title, §à§ã§ä§Ñ§Ó§Ý§Ö§ß §ä§à§Ý§î§Ü§à §ï§Ý§Ö§Þ§Ö§ß§ä &lt;title&gt;<br>
* §µ§Ò§â§Ñ§ß§í §Ó§ã§Ö §à§Ô§â§Ñ§ß§Ú§é§Ö§ß§Ú§ñ §ß§Ñ §Ú§ã§á§à§Ý§î§Ù§à§Ó§Ñ§ß§Ú§Ö &lt;empty-line&gt; §á§Ö§â§Ó§í§Þ §ï§Ý§Ö§Þ§Ö§ß§ä§à§Þ §Ó §Ü§à§ß§ä§Ö§Û§ß§Ö§â§Ö.<br>
* §¥§à§Ò§Ñ§Ó§Ý§Ö§ß§í §Ô§Ú§á§Ö§â§ã§ã§í§Ý§Ü§Ú, §á§à§ç§à§Ø§Ú§Ö §ß§Ñ html: &lt;a xlink:href="#ref"&gt;<br>
* §µ§Ò§â§Ñ§ß §ï§Ý§Ö§Þ§Ö§ß§ä &lt;note&gt;, §Ó§Þ§Ö§ã§ä§à §ß§Ö§Ô§à §ã§Ý§Ö§Õ§å§Ö§ä §á§à§Ý§î§Ù§à§Ó§Ñ§ä§î§ã§ñ §ã§ã§í§Ý§Ü§Ñ§Þ§Ú §Ó§Ú§Õ§Ñ &lt;a xlink:href="#note1" type="note"&gt;<br>
* §¿§Ý§Ö§Þ§Ö§ß§ä§à§Ó &lt;body&gt; §ä§Ö§á§Ö§â§î §Þ§à§Ø§Ö§ä §Ò§í§ä§î §ß§Ö§ã§Ü§à§Ý§î§Ü§à, §ã§ß§à§ã§Ü§Ú §ã§Ý§Ö§Õ§å§Ö§ä §â§Ñ§Ù§Þ§Ö§ë§Ñ§ä§î §Ó§ß§å§ä§â§Ú §Õ§à§á§à§Ý§ß§Ú§ä§Ö§Ý§î§ß§í§ç body<br>
* §¥§à§Ò§Ñ§Ó§Ý§Ö§ß §ï§Ý§Ö§Þ§Ö§ß§ä &lt;stylesheet&gt; §á§Ö§â§Ö§Õ &lt;description&gt; §Õ§Ý§ñ §Ù§Ñ§Õ§Ñ§ß§Ú§ñ §ä§Ñ§Ò§Ý§Ú§è §ã§ä§Ú§Ý§Ö§Û<br>
<br>
§Á §á§à§Õ§Ô§à§ä§à§Ó§Ú§Ý §ß§à§Ó§å§ð §à§ä§Ü§à§Þ§Þ§Ö§ß§ä§Ú§â§à§Ó§Ñ§ß§ß§å§ð §ã§ç§Ö§Þ§å §Ú §á§â§Ú§Þ§Ö§â §ã§à§à§ä§Ó§Ö§ä§ã§ä§Ó§å§ð§ë§Ö§Ô§à §Ö§Û §Õ§à§Ü§å§Þ§Ö§ß§ä§Ñ:<br>
<a href="files/FictionBook.xsd">FictionBook.xsd</a>,
<a href="files/FictionBookLinks.xsd">FictionBookLinks.xsd</a>,
<a href="files/demo.fb2">demo.fb2</a>.
<br>
§´§Ñ§Ü§Ø§Ö §Ö§ã§ä§î <a href="FictionBook_description.html">§à§á§Ú§ã§Ñ§ß§Ú§Ö FB2.0</a> §ß§Ñ §Ñ§ß§Ô§Ý. §ñ§Ù§í§Ü§Ö.



      <hr noshade="1">
      <div class="update">§±§à§ã§Ý§Ö§Õ§ß§Ö§Ö §à§Ò§ß§à§Ó§Ý§Ö§ß§Ú§Ö: 2002/08/06 21:04</div>
      §¡§Ó§ä§à§â: <a href="mailto:mike@haali.net">§®§Ú§ç§Ñ§Ú§Ý §®§Ñ§è§ß§Ö§Ó, mike@haali.net</a>
    </div>
  </body>
</html>