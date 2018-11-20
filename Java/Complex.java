
public class Complex {
	public double re;
	public double im;
	
	public Complex() {
		this.re = 0.0;
		this.im = 0.0;
	}
	public Complex(double real, double imag) {
		this.re = real;
		this.im = imag;
	}
	
	Complex mult(Complex r) {
		Complex result = new Complex();
		result.re = this.re * r.re - this.im * r.im;
		result.im = this.re * r.im + this.im * r.re;
		return result;
	}

	Complex add(Complex r) {
		Complex result = new Complex();
		result.re = this.re + r.re;
		result.im = this.im + r.im;
		return result;
	}
}