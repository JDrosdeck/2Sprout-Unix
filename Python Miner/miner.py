import threading,os,Queue,time,HTMLParser, string, re
#i'm not sure about the following items
import socket, sys, struct, time
from xml.dom.minidom import Document
from BeautifulSoup import BeautifulSoup
from htmlentitydefs import name2codepoint as n2cp
from urllib import urlopen


#url = "http://www.amazon.com/gp/product/B001K2D36K/ref=vg_flx_newarrivals?pf_rd_p=467196411&pf_rd_s=center-5&pf_rd_t=101&pf_rd_i=229575&pf_rd_m=ATVPDKIKX0DER&pf_rd_r=0GWQKCFKJ3NEX1SB0TD1"
url = "http://www.lowes.com/lowes/lkn?action=productDetail&productId=228957-53458-SC1920-BSN"
page_html = urlopen(url).read()
page_html = page_html.lower()




#This is what works for lowes.com

matchword = "<b class=\"txtlg\"[^>]*><b>" + "([^<]+)" +"</b>"
p = re.compile(matchword)
m = p.findall(page_html)

#d<b class="txtlg"><b>$2.54</b>






#below is whats used for amazon.com It just scrapes the list prices and the actual prices
"""
matchword = "<b class=\"price\"[^>]*>" + "([^<]+)" +"</b>"
p = re.compile(matchword)
m = p.findall(page_html)

matchListPrice = "<td class=\"listprice\"[^>]*>" + "([^<]+)" +" </td>"
g = re.compile(matchListPrice)
i = g.findall(page_html)

#<td class="listprice">$29.99 </td>
"""



BSoup = BeautifulSoup(page_html)

# title
title = BSoup.html.head.title.string

# description
description = BSoup.head.find("meta",{"name":"description"})
if description and 'content' in dict(description.attrs):
	description = description['content']

# keyword
keywords = BSoup.head.find("meta",{"name":"keywords"})
if keywords and 'content' in dict(keywords.attrs):
	keywords = keywords['content']         

# medium
medium = BSoup.head.find("meta",{"name":"medium"})
if medium and 'content' in dict(medium.attrs):
	medium = medium['content']


doc = Document()  
sproutcast = doc.createElement("sproutcast")
doc.appendChild(sproutcast)

urlNode = doc.createElement("url")
sproutcast.appendChild(urlNode)

urlData = doc.createTextNode(url)
urlNode.appendChild(urlData)    

titleNode = doc.createElement("title")
sproutcast.appendChild(titleNode)

titleData = doc.createTextNode(title)
titleNode.appendChild(titleData)

#ListNode = doc.createElement("listPrice")
#sproutcast.appendChild(ListNode)

#ListData = doc.createTextNode(i[0])
#ListNode.appendChild(ListData)

priceNode = doc.createElement("Price")
sproutcast.appendChild(priceNode)

priceData = doc.createTextNode(m[0])
priceNode.appendChild(priceData)




if description != None:
   descriptionNode = doc.createElement("description")
   sproutcast.appendChild(descriptionNode)

   descriptionData = doc.createTextNode(description)
   descriptionNode.appendChild(descriptionData)

if keywords != None:
   keywordsNode = doc.createElement("keywords")
   sproutcast.appendChild(keywordsNode)

   keywordsData = doc.createTextNode(keywords)
   keywordsNode.appendChild(keywordsData) 

if medium != None:
   mediumNode = doc.createElement("medium")
   sproutcast.appendChild(mediumNode)

   mediumData = doc.createTextNode(medium)
   mediumNode.appendChild(mediumData)

msg = doc.toxml()
print msg











     #msg = keywords


#<b class="price">$27.99</b><span class="plusShippingText">&nbsp;+&nbsp;$2.95&nbsp;shipping</span><br />




       