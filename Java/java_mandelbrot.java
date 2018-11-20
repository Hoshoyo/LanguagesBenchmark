import java.awt.image.BufferedImage;
import java.io.File;
import javax.imageio.ImageIO;
import java.io.IOException;

public class java_mandelbrot {

	public static void main(String[] args){
		int width = 800;
		int height = 800;
		
		double scale = 2.0;
		double xoff = 0.0;
		double yoff = 0.0;
		int its = 256;

		long startTime = System.nanoTime();
		
		BufferedImage bufferedImage = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);
		
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {

				double half_scale = scale / 2.0;

				double x_coord = ((double)x / width)  * scale - half_scale;
				double y_coord = ((double)y / height) * scale - half_scale;

				x_coord += xoff;
				y_coord += yoff;

				int r = 0;
				int g = 0;
				int b = 0;

				Complex z = new Complex();
				Complex coord = new Complex(x_coord, y_coord);
				
				for (int it = 0; it < its; it++) {
					z = z.mult(z).add(coord);
					double v = z.re * z.re + z.im * z.im;
					if (Math.sqrt(v) > 2.0) {
						r = it;
						g = it;
						b = it;
						break;
					}
				}

				bufferedImage.setRGB(x, y, 0xff000000 | (r << 16) | (g << 8) | b);
			}
		}
		long endTime   = System.nanoTime();
		long totalTime = endTime - startTime;
		System.out.println("Total time elapsed: " + totalTime / 1000000.0 + " ms");
		
		
		try {
			// retrieve image
			String imageFilename = "java_mandelbrot_image.png";
			File outputfile = new File(imageFilename);
			ImageIO.write(bufferedImage, "png", outputfile);
		} catch (IOException e) {
			System.out.println("Could not write file");
		}
	}
}