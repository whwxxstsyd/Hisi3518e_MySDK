/* 自研OCX插件 */
function OCX(){
	var hDevice;
	this.LoadXX = function(id, count){
		//$(id).empty();
		
		var td_width;
		var td_height;
		var col;
		
		if(count == 1){
			col = 1;
			td_width = $(id).width()-6;
			td_height = $(id).height()-6;
		}else if(count == 4){
			col = 2;
			td_width = $(id).width()/2-5;
			td_height = $(id).height()/2-5;
		}else if(count == 9){
			col = 3;
			td_width = $(id).width()/3-4;
			td_height = $(id).height()/3-4;
		}
		var playElements = "";
		if (GetBrowserVer().brower == "IE"){      
		}else{		
/*			
			playElements += '<object clsid="{D7108685-7911-49CC-92C0-CEFEF981A338}" ';
			playElements += 'progid="TVSWebClientPlus.ocx#version=3,0,4,9" ';
			playElements += 'width="'+(td_width-2)+'" height="'+(td_height-2)+'"';
			playElements += 'type="application/x-tvsocx-plugin" viewastext>';
			playElements += '<div id="control_error"><p id="control_head">&nbsp;</p><br/><p id="control_content1">&nbsp;</p><br/><p id="control_content2">&nbsp;</p><br/><p id="control_download"><strong><a href="TVSClientOCX_Setup.exe">&nbsp;</a></strong></p></div></object>';		
*/			
			playElements += '<object width="704" height="576" classid="clsid:D7108685-7911-49CC-92C0-CEFEF981A338" >';
			playElements += '<embed id="player" type="application/ipcam-plugin" width=960 height=540 ></embed></object>';
		}		
		var str = "<table>";
		for(var i=0;i<col;i++){
			str += '<tr>';
			for(var j=0;j<col;j++){
				/*
				if(i==0 && j==0){
					str += '<td width="'+td_width+'" height="'+td_height+'">'+playElements+'<div class="snap"><img src="/web/tmpfs/snap.jpg" /></div></td>';
				}else{
					str += '<td width="'+td_width+'" height="'+td_height+'">'+playElements+'</td>';
				}*/
				str += '<td width="'+td_width+'" height="'+td_height+'">'+playElements+'</td>';
			}
			str += '</tr>';
		}
		str += "</table>";
		
		if (GetBrowserVer().brower != "IE")
			$(id).html(str);
	}
	this.Load = function(id, count)
	{

	}
	
	this.getPlayObj = function(){
			return document.getElementById("player");	
	}
	
	this.Play = function(stream_index){
		var playObjs = this.getPlayObj();
		var string = window.location.host; 
		var StrArray = new Array();
		
		StrArray = string.split(":");
		try{	
			hDevice = playObjs.DeviceLogin(3, StrArray[0], uname, upwd, parseInt(StrArray[1]), 3);		
		  playObjs.StartRealPlay(hDevice, 0, 0);		
		}catch(err) {
			$("#PlayBox").html('<a target="_blank" class="downplayer" href="/setup.exe">'+str_plugin+'</a>');
		}	  	
	  /*	去掉多画面
		setTimeout(function(){
			var cgiurl = "cgi-bin/hi3510/param.cgi?cmd=getuserinfo&cmd=getdevices";
			$.get(cgiurl,function(data){
				eval(data);
				for(var i=0;i<playObjs.length;i++){
					var hDevice
					if(i == 0){				// 本机
						hDevice = playObjs[i].DeviceLoginExt(3, window.location.host, uname, upwd, window.location.port, 3);
					}else{
						hDevice = playObjs[i].DeviceLoginExt(3, ip[i], user[i], pwd[i], port[i], 3);
					}
					playObjs[i].StartRealPlay(hDevice, 0, 0);					
				}
			});
		},100);
		*/
	}
	this.ptzctrl = function() {}
	this.enablePic = function() {}
	this.openMdSetPage = function() {}
	this.getPic = function() {}
	this.snap = function() {}
	this.setRecordPath = function() {}
	this.getRecordPath = function() {}
	this.zoomin = function() {}
	this.zoomout = function() {}
	this.talk = function(start) {
			var playObjs = this.getPlayObj();		
			if (start)
				playObjs.StartTalk(hDevice);
			else
				playObjs.StopTalk(hDevice);
		}
	this.listen = function(start) {
			var playObjs = this.getPlayObj();				
			if (start)
				playObjs.StartListen(hDevice);
			else
				playObjs.StopListen(hDevice);		
		}
	this.record = function() {}
	this.fullscreen = function() {}
}

/* MAC下使用QuickTime播放 */
function QT() 
{
		this.LoadXX = function(id) 
		{
				var cgiurl = "/cgi-bin/hi3510/getrtsplisnport.cgi";
				var port = 554;
				
				$.ajax({   
						url:cgiurl,   
						type: "GET",   
						dataType:"text",  
						timeout: 3000,   
						error: function(data){},
						success: function(data){eval(data); port = rtsplisnport;},
						async:false
				})
				switch (getPlayStream()) 
				{
					case '11':
							playchn = 11;
							break;
					case '12':		
							default:
							playchn = 12;
							break;
				}
				
				
				str = '<embed id="QTPlayer" src="rtsp://' + document.location.hostname + ':' + 554 + '/' + '11' +'" ';
				str += 'width="'+$(id).width()+'px" height="'+$(id).height()+'px" ';
				str += 'autoplay="true" controller="true" scale="tofit" showstatusbar="false" type="video/quicktime" ';
				str += 'pluginspage="http://www.apple.com/quicktime/"></embed>';
				$(id).html(str);
    }
    this.Load = function(id) 
    {
    	
    }
    this.play = function(id) 
   	{
        return true;
    }
    this.StopPlay = function() 
   	{
        try {
						document.getElementById("QTPlayer").Stop();
        } catch(e) {
            return false;
        }
    }
}

/* 使用VLC播放 */
function VLC() 
{
    this.Load = function(id) 
    {
	str = '<object classid="clsid:9BE31822-FDAD-461B-AD51-BE1D1C159921" id="VLCPlayer" ';
        str += 'width="' + $(id).width() + 'px" height="' + $(id).height() + 'px" events="True" wmode="transparent" quality="high">';
        str += '<param name="MRL" value="" />';
        str += '<param name="Src" value="" />';
        str += '<param name="ShowDisplay" value="True" />';
        str += '<param name="AutoLoop" value="False" />';
        str += '<param name="AutoPlay" value="False" />';
        str += '<param name="Time" value="True" />';
        str += '<param name="toolbar" value="false">';
				str += '<param name="wmode" value="opaque" />';
        str += '<embed pluginspage="http://www.videolan.org" type="application/x-vlc-plugin" wmode="transparent" quality="high" ';
        str += ' version="VideoLAN.VLCPlugin.2" ';
        str += ' width=" ' + $(id).width() + ' px" height="' + $(id).height() + 'px" ';
        str += 'toolbar="false" ';
        str += ' text="Waiting for video" name="VLCPlayer" wmode="transparent"> </embed> ';
        str += '</object>';
        $(id).html(str);
    };
		this.getVLC = function(name) 
		{
				if (window.document[name])
						return window.document[name];
				if (navigator.appName.indexOf("Microsoft Internet") == -1)
						if (document.embeds && document.embeds[name]) return document.embeds[name];
				else
						return document.getElementById(name);
		};
    this.play = function(id) 
   	{
				var port = 554;
				var playchn = 10;
				var vlc = this.getVLC("VLCPlayer");
				
				setTimeout(function() {
						try 
						{
								itemId = vlc.playlist.add("rtsp://" + document.location.hostname + ":" + port + "/" + playchn);
								vlc.playlist.playItem(itemId);
						} catch(e) {
								$(id).html('<a target="_blank" class="downplayer" href="http://www.videolan.org/">'+str_error_control_download+'</a>');
						}
				},100);
		};
		this.StopPlay = function() 
		{
				try 
				{
						var vlc = this.getVLC;
						
						vlc.playlist.stop();
				} catch(e) {
						return false;
				}
		};
		this.RePlay = function(id)
		{
				this.getVLC("VLCPlayer").playlist.stop();
				this.play(id);
				return;
		};
		this.PlayerResize = function(width, height) 
		{
				try 
				{
						var vlc = this.getVLC;
						
						vlc.style.width = width;
						vlc.style.height = height;
				} catch(e) {
						return false;
				}
		}
}

/* 根据浏览器选择播放对象 */
function PlayerObj() 
{
    var Player;
    
		if(OS()=="Mac")
				Player = new QT();
		else
		{
			Player = new OCX();	
		}
		this.Load = function(id) {return Player.Load(id, 1);}
		this.Play = function(stream_index) {return Player.Play(stream_index);}
		this.RePlay = function(id) {return Player.RePlay(id);}
		this.StopPlay = function() {return Player.StopPlay();}
		this.PlayerResize = function(w, h) {return Player.PlayerResize(w, h);}
		this.talk = function(start) {return Player.talk(start);}		
		this.listen = function(start) {return Player.listen(start);}				
}