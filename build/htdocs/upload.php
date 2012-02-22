<form action="/uhook/time" method="post" enctype="multipart/form-data">
<input type="file" name="f" />
<input type="submit" value="Send" />
</form>
<?php
if (count($_FILES)) {
  $tmp_name = $_FILES["f"]["tmp_name"];
  $name = $_FILES["f"]["name"];
  move_uploaded_file($tmp_name, "$name");
}
?>
<pre>
<?php
if (isset($_FILES)) {
  var_dump($_FILES);
}
?>
</pre>
