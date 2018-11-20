close all;
clear all;

width = 800;
height = 800;
im = zeros(width, height, 3);

scale = 2.0;
xoff = 0.0;
yoff = 0.0;
its = 256;

tic

for y = 1:height
    for x = 1:width
        half_scale = scale / 2.0;

        x_coord = (x / width)  * scale - half_scale;
        y_coord = (y / height) * scale - half_scale;

        x_coord = x_coord + xoff;
        y_coord = y_coord + yoff;

        r = 0;
        g = 0;
        b = 0;

        z = complex(0, 0);
        coord = complex(x_coord, y_coord);

        for it = 1:its
            z = z * z + coord;
            if (sqrt(real(z) * real(z) + imag(z) * imag(z)) > 2.0)
                r = (it - 1) / 255.0;
                g = r;
                b = r;
                break;
            end
        end

        im(y, x, :) = [r, g, b];
    end
end
toc

imwrite(im, 'matlab_mandelbrot.png');