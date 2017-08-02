<html>
<!- Copyright (c) Go Ahead Software Inc., 1999-2010. All Rights Reserved. ->
<head>

<!-- del by gyr 2011.10.15
<title>ASP Test Page</title>
-->
<title> new document </title> <!-- add by gyr 2011.10.15 -->

</head>

<body>

<span style="color:#3333ff;"><!-- added start for test upload file by gyr 2011.10.15 -->
<h1>GoForm upload file test</h1>
<form id="softupdate" action=/goform/formUploadFileTest method=POST enctype="multipart/form-data">
	<table>
		Select file: <td> <input id="fileupload" type="file" name="fileupload" size=60 value=""> 	</td>
		<td> <input id="fileuploadsubmit" type="submit" name="update" value="update" onClick="uploadFileSubmit()">	</td>
	</table>
</form>
<!-- added end for test upload file by gyr 2011.10.15 -->

</span>

</body>
</html>