// JavaScript Document

var bMouseDown;
var bDraw;
var arearow, areacol;
var areaselectId;//控件的ID
var arearcellwidth;//格子的宽
var areacellheigh;//格子的高
var drawcolor="#ff0000";
var selectedcolor="#4791c5";//选中时格子的颜色
var boder="1px solid #dbf0ff";//边框的样式

function AreaSelectOnMouseDown(evt)
{
	bMouseDown = 1;

	evt = evt||window.event;
	
	var obj = (evt.srcElement ? (evt.srcElement) : evt.target);
	
	/*这次拖动以鼠标按下的第一个格子的颜色 为准，即格子颜色为红色，则这次手动全部把格式变成透明*/
	if((obj)&&(obj.tagName == "DIV")&&(obj.id))
	{
		var spanobj = document.getElementById(obj.id);
		
		/*兼容 firefox chrome*/
		if(spanobj.style.backgroundColor.split('rgb'))
				var sHexColor = spanobj.style.backgroundColor.colorHex();
		else
				var sHexColor = spanobj.style.backgroundColor;

		/*反选颜色*/		
		if(sHexColor == selectedcolor)
		{
			drawcolor="transparent";	
		}
		else
		{
			drawcolor= selectedcolor;
		}
	}
}

function AreaSelectOnMouseUp(evt)
{
	bMouseDown = 0;	
}
 
 
function AreaSelectOnMouseMove(evt)
{
	if((bMouseDown)&&(!bDraw))//为避免重复画，bDraw控制方框只画一次
	{
		/*兼容firefox*/
		evt = evt||window.event;
		var obj = (evt.srcElement ? (evt.srcElement) : evt.target);
		
		if((obj)&&(obj.tagName == "DIV")&&(obj.id))
		{
			var spanobj = document.getElementById(obj.id);
			/*反选颜色*/		
			spanobj.style.backgroundColor=drawcolor;	
			bDraw = 1;	
		}
	}
}

function AreaSelectOnMouseOut(evt)
{
	bDraw = 0;	
}

function AreaSelectOndblclick(evt)
{
	/*兼容firefox*/
	evt = evt||window.event;
	var obj = (evt.srcElement ? (evt.srcElement) : evt.target);
	
	if((obj)&&(obj.tagName == "DIV")&&(obj.id))
	{
		var spanobj = document.getElementById(obj.id);
		
		/*兼容 firefox chrome*/
		if(spanobj.style.backgroundColor.split('rgb'))
		{
				var sHexColor = spanobj.style.backgroundColor.colorHex();
		}
		else
		{
				var sHexColor = spanobj.style.backgroundColor;
		}

		/*反选颜色*/		
		if(sHexColor==selectedcolor)
		{
			spanobj.style.backgroundColor = "transparent";	
		}
		else
			spanobj.style.backgroundColor = selectedcolor;	
	}
}
   
function AreaselectSelectall()
{
	for(i=0; i<arearow; i++)//行
	{
		for(j=0;j<areacol;j++)//列
		{
				spanid='Row'+i+'Col'+j;	
				var spanobj = document.getElementById(spanid);
				if(spanobj)
					spanobj.style.backgroundColor=selectedcolor;
		}
	}	
}

function AreaselectClearall()
{
	for(i=0; i<arearow; i++)//行
	{
		for(j=0;j<areacol;j++)//列
		{
				spanid='Row'+i+'Col'+j;	
				var spanobj = document.getElementById(spanid);
				if(spanobj)
					spanobj.style.backgroundColor="transparent";
		}
	}	
}

function AlearselectDraw(col, row, LayerWidth, LayerHeigh, id){
	arearow = row;
	areacol =col;
	areaselectId = id;
	cellwidth = arearcellwidth = parseInt(LayerWidth/col) ;
	cellheigh = areacellheigh = parseInt(LayerHeigh/row);

	document.open();
	document.write('<div id="' +areaselectId+'" >');

	for(i=0; i<arearow; i++)//行
	{
		rowid = 'Row' + i;
		document.write('<div id="' +rowid+'" >');//一行开始
		document.getElementById(rowid).style.width = (cellwidth*areacol)+'px';
	
		for(j=0;j<areacol;j++)//列
		{
			spanid='Row'+i+'Col'+j;	
		
			/*针对方框在不同位置，它四周的边框不一样，如第一行第一列，它的边框就要4个，
			第一行其它列，它的边框就只要3个，上，右，下*/
			if((i==0)&&(j==0))
			{
				document.write('<div id='+spanid+'></div>');//第一行第一列
				document.getElementById(spanid).style.border = boder;
				document.getElementById(spanid).style.width = arearcellwidth-2+'px';
				document.getElementById(spanid).style.height = areacellheigh-2+'px';
			}
			else if((i==0)&&(j>0))	
			{
				document.write('<div id='+spanid+'></div>');//第一行其它列
				document.getElementById(spanid).style.borderTop= boder;
				document.getElementById(spanid).style.borderBottom= boder;
				document.getElementById(spanid).style.borderRight=boder;
				document.getElementById(spanid).style.width = arearcellwidth-1+'px';
				document.getElementById(spanid).style.height = areacellheigh-2+'px';
			}
			else if((i>0)&&(j==0))		
			{
				document.write('<div id='+spanid+'></div>');//其它行第一列
				document.getElementById(spanid).style.borderLeft= boder;
				document.getElementById(spanid).style.borderBottom= boder;
				document.getElementById(spanid).style.borderRight=boder;
				document.getElementById(spanid).style.width = arearcellwidth-2+'px';
				document.getElementById(spanid).style.height = areacellheigh-1+'px';
			}
			else
			{
				document.write('<div id='+spanid+'></div>');//其它行其它列
				document.getElementById(spanid).style.borderBottom= boder;
				document.getElementById(spanid).style.borderRight=boder;
				document.getElementById(spanid).style.width = arearcellwidth-1+'px';
				document.getElementById(spanid).style.height = areacellheigh-1+'px';
			}
			document.getElementById(spanid).style.cssFloat = "left";
			document.getElementById(spanid).style.styleFloat = "left";
			document.getElementById(spanid).style.margin = "0px";

		}
		document.write('</div>');//一行结束
		
	}
	document.write('</div>');//end draw
	document.close();

	/*********************注册消息函数*********************/
	document.getElementById(areaselectId).onmousemove=AreaSelectOnMouseMove;
	document.getElementById(areaselectId).onmousedown=AreaSelectOnMouseDown;
	document.getElementById(areaselectId).onmouseup=AreaSelectOnMouseUp;
	document.getElementById(areaselectId).onmouseout=AreaSelectOnMouseOut;
	document.getElementById(areaselectId).ondblclick = AreaSelectOndblclick;
}

/****************设置某行某列的值**************/
function SetAlearselect(rowno, colno, colbit)   
{
	if((rowno > arearow)||(colno > areacol))
		return false;
		
	spanid='Row'+rowno+'Col'+colno;	
	var spanobj = document.getElementById(spanid);
	if(spanobj)
	{
		if( colbit & 1)
			spanobj.style.backgroundColor=selectedcolor;
		else
			spanobj.style.backgroundColor="transparent";	
	}
	
}
/****************获取某行某列的值**************/
function GetAlearselect(rowno, colno)   
{
	if((rowno > arearow)||(colno > areacol))
		return false;
	spanid='Row'+rowno+'Col'+colno;	
	var spanobj = document.getElementById(spanid);
	if(spanobj){
		var sHexColor;
		/*兼容 firefox chrome*/
		if(spanobj.style.backgroundColor.split('rgb'))
			var sHexColor = spanobj.style.backgroundColor.colorHex();
		else
			var sHexColor = spanobj.style.backgroundColor;
		if( sHexColor== selectedcolor)
			return 1;
		else
			return 0;
	}
	
	return 0;
} 

//十六进制颜色值域RGB格式颜色值之间的相互转换
//-------------------------------------
//十六进制颜色值的正则表达式
var reg = /^#([0-9a-fA-f]{3}|[0-9a-fA-f]{6})$/;
/*RGB颜色转换为16进制*/
String.prototype.colorHex = function() {
    var that = this;
    if (/^(rgb|RGB)/.test(that)) {
        var aColor = that.replace(/(?:\(|\)|rgb|RGB)*/g, "").split(",");
        var strHex = "#";
        for (var i = 0; i < aColor.length; i++) {
            var hex = Number(aColor[i]).toString(16);
            if (hex === "0") {
                hex += hex;
            }
            strHex += hex;
        }
        if (strHex.length !== 7) {
            strHex = that;
        }
        return strHex;
    } else if (reg.test(that)) {
        var aNum = that.replace(/#/, "").split("");
        if (aNum.length === 6) {
            return that;
        } else if (aNum.length === 3) {
            var numHex = "#";
            for (var i = 0; i < aNum.length; i += 1) {
                numHex += (aNum[i] + aNum[i]);
            }
            return numHex;
        }
    } else {
        return that;
    }
};

//-------------------------------------------------
/*16进制颜色转为RGB格式*/
String.prototype.colorRgb = function() {
    var sColor = this.toLowerCase();
    if (sColor && reg.test(sColor)) {
        if (sColor.length === 4) {
            var sColorNew = "#";
            for (var i = 1; i < 4; i += 1) {
                sColorNew += sColor.slice(i, i + 1).concat(sColor.slice(i, i + 1));
            }
            sColor = sColorNew;
        }
        //处理六位的颜色值
        var sColorChange = [];
        for (var i = 1; i < 7; i += 2) {
            sColorChange.push(parseInt("0x" + sColor.slice(i, i + 2)));
        }
        return "RGB(" + sColorChange.join(",") + ")";
    } else {
        return sColor;
    }
};