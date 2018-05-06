#ifndef mmocBASIC_MATH_HPP
#define mmocBASIC_MATH_HPP

#include <cmath>
#include <SFML/System/Vector2.hpp>

namespace mmoc
{

//checks if point p is on left of (but not on) line drawn from l_a to l_b extended endlessly
template<typename T> bool point_on_left(sf::Vector2<T> p,sf::Vector2<T> l_a,sf::Vector2<T> l_b)
	{ return (l_b.x-l_a.x)*(p.y-l_a.y) - (l_b.y-l_a.y)*(p.x-l_a.x) > 0; }

//can store only proper rotations and scalings; no mirrors (a.k.a improper rotations), skews, etc.
template<typename T> struct CoordSys2D
{
	//always formed from x basis, then y basis is a quarter turn ACW
	T x,y;
	
	CoordSys2D(T x_in = 1,T y_in = 0)
		: x(x_in), y(y_in) {}
	CoordSys2D(sf::Vector2<T> x_basis)
		: x(x_basis.x), y(x_basis.y) {}
	
	sf::Vector2<T> x_basis() const
		{ return sf::Vector2<T>(x,y); }
	sf::Vector2<T> y_basis() const
		{ return sf::Vector2<T>(-y,x); }
	
	CoordSys2D inverse() const
		{ return CoordSys2D(x,-y)/(x*x+y*y); }
};
//matrix & scalar
template<typename T> CoordSys2D<T> operator* (T scalar,CoordSys2D<T> coordsys)
	{ return CoordSys2D<T>(scalar*coordsys.x_basis()); }
template<typename T> CoordSys2D<T> operator* (CoordSys2D<T> coordsys,T scalar)
	{ return scalar*coordsys; }
template<typename T> CoordSys2D<T> operator/ (CoordSys2D<T> coordsys,T scalar)
	{ return CoordSys2D<T>(coordsys.x_basis()/scalar); }
//matrix & vector
template<typename T> sf::Vector2<T> operator* (CoordSys2D<T> coordsys,sf::Vector2<T> vec)
	{ return coordsys.x_basis()*vec.x + coordsys.y_basis()*vec.y; }
template<typename T> sf::Vector2<T> operator* (sf::Vector2<T> vec,CoordSys2D<T> coordsys)
	{ return coordsys*vec; }
template<typename T> sf::Vector2<T> operator/ (sf::Vector2<T> vec,CoordSys2D<T> coordsys)
	{ return coordsys.inverse()*vec; }
//matrix & matrix
template<typename T> CoordSys2D<T> operator* (CoordSys2D<T> left,CoordSys2D<T> right)
	{ return CoordSys2D<T>(right.x*left.x_basis() + right.y*left.y_basis()); }
template<typename T> CoordSys2D<T> operator/ (CoordSys2D<T> left,CoordSys2D<T> right)
	{ return left*right.inverse(); }

} //end of namespace mmoc

#endif