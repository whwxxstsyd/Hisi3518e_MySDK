var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

$(function()			
{
	get_value();
	$("#save").bind("click", set_value);
});

function DEVNAME_MODEL()
{
	this.devname = "";
	
	this.getvalue = function()
	{
		var res=null;
		var cgiurl="/cgi-bin/get_param.cgi?camera_name&user="+uname+"&pwd="+upwd;
		$.ajax({
			url:cgiurl,
			async:false,
			cache:false,
			success: function(data){
				eval(data);
				res={
					"devname":camera_name,				//摄像机名字
				};
			}
		});
		return res;	
	}	
	
	this.setvalue=function()
	{
		var res=null;
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd;
		cgiurl+="&camera_name="+encodeURIComponent(this.devname);
		$.get(cgiurl);
	}	
}

function get_value()
{
	var model = new DEVNAME_MODEL();
	var obj = model.getvalue();
 	$("#devname").val(obj.devname);
}

function set_value()
{
	var model = new DEVNAME_MODEL();
	model.devname=$("input[id=devname]").val();
	model.setvalue();
	save_tips(str_save_succeed);	
}