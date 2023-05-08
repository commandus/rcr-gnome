# glib-compile-resources
$GLIB_COMPILE_RESOURCES = $($args[0])
$CMAKE_CURRENT_SOURCE_DIR = $($args[1])
Write-Output($CMAKE_CURRENT_SOURCE_DIR)
New-Item -ItemType Directory -Force -Path $CMAKE_CURRENT_SOURCE_DIR | Out-Null

$C="$GLIB_COMPILE_RESOURCES $CMAKE_CURRENT_SOURCE_DIR/rcr-gnome.resource.xml --target=$CMAKE_CURRENT_SOURCE_DIR/resources.c --generate-source"
Write-Output($C)
Invoke-expression $C


