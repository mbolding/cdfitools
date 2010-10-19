xdoc = xmlread('../psychoquartz/stroopscript.xml');
colorListItems = xdoc.getElementsByTagName('color');
wordListItems = xdoc.getElementsByTagName('word');
for k = 0:allListItems.getLength-1
    theword = char(wordListItems.item(k).getTextContent);
    thecolor = char(colorListItems.item(k).getTextContent);   
    disp([theword '  ' thecolor]);
end