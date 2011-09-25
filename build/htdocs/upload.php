<form action="" method="post" enctype="multipart/form-data">
<input type="file" name="f" />
<input type="submit" value="Send" />
</form>
<?php
if (count($_FILES)) {
  $tmp_name = $_FILES["f"]["tmp_name"];
  $name = $_FILES["f"]["name"];
  move_uploaded_file($tmp_name, "$name");
}
if (isset($_POST)) {
  var_dump($_POST);
}
?>
