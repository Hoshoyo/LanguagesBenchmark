<?php

ini_set('memory_limit', '-1');

$width = 800;
$height = 800;

// Create a blank image and add some text
$im = imagecreatetruecolor($width, $height);
$text_color = array();
for($i = 0; $i < 256; $i++){
	$text_color[$i] = imagecolorallocate($im, $i, $i, $i);
}

$black = imagecolorallocate($im, 0, 0, 0);

$its = 256;
$scale = 2.0;
$xoff = 0.0;
$yoff = 0.0;

class Complex{
	public $re;
	public $im;
		
	function __construct($re, $im){
		$this->re = $re;
		$this->im = $im;
	}
	
	function add($l, $r){
		$result = new Complex(0.0, 0.0);
		$result->re = $l->re + $r->re;
		$result->im = $l->im + $r->im;
		return $result;
	}
	function mult($l, $r){
		$result = new Complex(0.0, 0.0);
		$result->re = $l->re * $r->re - $l->im * $r->im;
		$result->im = $l->re * $r->im + $l->im * $r->re;
		return $result;
	}
}

$starttime = microtime(true);


for($y = 0; $y < $width; $y++){
	for($x = 0; $x < $height; $x++){

		$half_scale = $scale / 2.0;

			$x_coord = (round($x) / $width)  * $scale - $half_scale;
			$y_coord = (round($y) / $height) * $scale - $half_scale;

			$x_coord += $xoff;
			$y_coord += $yoff;

			$z = new Complex(0.0, 0.0);
			$coord = new Complex($x_coord, $y_coord);
			
			$it = 0;
			
			$use_color = $black;

			for (; $it < $its; $it++) {
				$z = $z->add($z->mult($z, $z), $coord);
				$v = $z->re * $z->re + $z->im * $z->im;
				if (sqrt($v) > 2.0) {
					$use_color = $text_color[$it];
					break;
				}
			}
			unset($z);
			unset($coord);

			imagesetpixel($im, $x, $y, $use_color);
	}
}

/* do stuff here */
$endtime = microtime(true);
$timediff = $endtime - $starttime;


// Set the content type header - in this case image/jpeg
header('Content-Type: image/jpeg');

// Output the image
imagepng($im);

// Free up memory
imagedestroy($im);

echo $timediff;

?>
