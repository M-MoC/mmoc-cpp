#include <string>
#include <SFML/Graphics.hpp>
#include "maths_utils.hpp"

int main()
{
	float fps=60.f,pixels_per_unit=50.f;
	sf::ContextSettings opengl_context;
	std::string base_title="interactive maths_utils test demo";
	opengl_context.antialiasingLevel=4;
	sf::RenderWindow window(sf::VideoMode(800,600),base_title,sf::Style::Default,opengl_context);
	
	sf::View view(sf::Vector2f(),sf::Vector2f(800,600));
	window.setView(view);
	
	sf::CircleShape point(4.f);
	point.setOrigin(4.f,4.f);
	mmoc::CoordSys2D<double> coordsys;
	sf::Vector2<double> vec(1.0,0.0);
	double coordsys_speed=0.4/fps,vec_speed=0.8/fps;
	
	sf::CircleShape unit_circle(pixels_per_unit-2.f);
	unit_circle.setFillColor(sf::Color::Transparent);
	unit_circle.setOutlineColor(sf::Color::Blue);
	unit_circle.setOutlineThickness(4.f);
	unit_circle.setOrigin(pixels_per_unit-1.f,pixels_per_unit-1.f);
	
	sf::ConvexShape static_convex_poly(6);
	static_convex_poly.setPoint(0,sf::Vector2f(0.f,0.f));
	static_convex_poly.setPoint(1,sf::Vector2f(1.f,0.f));
	static_convex_poly.setPoint(2,sf::Vector2f(1.f,1.f));
	static_convex_poly.setPoint(3,sf::Vector2f(0.f,2.f));
	static_convex_poly.setPoint(4,sf::Vector2f(-1.f,2.f));
	static_convex_poly.setPoint(5,sf::Vector2f(-1.f,1.f));
	static_convex_poly.setPosition(sf::Vector2f(2.5f,2.f)*pixels_per_unit);
	
	sf::ConvexShape convex_poly(static_convex_poly);
	convex_poly.setScale(sf::Vector2f(1.f,1.f)*pixels_per_unit);
	
	sf::Clock clock;
	while(window.isOpen())
	{
		sf::Event event;
		while(window.pollEvent(event))
		{
			switch(event.type)
			{
				case sf::Event::Closed:
					window.close();
					break;
				case sf::Event::Resized:
					view.setSize(sf::Vector2f(window.getSize()));
					window.setView(view);
					break;
				default: break;
			}
		}
		
		if(window.hasFocus())
		{
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
				coordsys.x-=coordsys_speed;
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
				coordsys.x+=coordsys_speed;
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
				coordsys.y-=coordsys_speed;
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
				coordsys.y+=coordsys_speed;
			
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
				vec.x-=vec_speed;
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
				vec.x+=vec_speed;
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
				vec.y-=vec_speed;
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
				vec.y+=vec_speed;
		}
		
		window.clear(sf::Color::Black);
		
		//for convenience - mouse_pos needs to be relative to centre of game world, which
		//due to the view, is actually in the middle of the window not the top-left corner
		sf::Vector2f mouse_pos( sf::Mouse::getPosition(window)-sf::Vector2i(window.getSize())/2 );
		
		//setting convex_poly's points to correspond internally with the transform coordsys applied
		//to the internal points of static_convex_poly
		for(int i=0;i<static_convex_poly.getPointCount();++i)
			convex_poly.setPoint( i, sf::Vector2f(
				coordsys*sf::Vector2<double>(static_convex_poly.getPoint(i))
				));
		//making convex poly highlight in green when mouse hovers over it
		std::vector<sf::Vector2f> pure_poly;
		for(int i=0;i<static_convex_poly.getPointCount();++i)
			pure_poly.push_back(convex_poly.getTransform()*convex_poly.getPoint(i));
		if(mmoc::in_convex_polygon<float,true>(mouse_pos,pure_poly))
			convex_poly.setFillColor(sf::Color(31,63,31));
		else convex_poly.setFillColor(sf::Color(31,31,31));
		
		window.draw(convex_poly);
		window.draw(unit_circle);
		
		//drawing transformed points
		auto draw_point=[&]()
		{
			sf::Vector2f temp=point.getPosition();
			point.setPosition(temp*pixels_per_unit);
			window.draw(point);
			point.setPosition(temp);
		};
		
		//set title of window to give info for what side of the x and y axes of coordsys
		//the cursor is currently on
		std::string new_title=base_title+" - ";
		//testing point_on_left()
		if(mmoc::point_on_left(mouse_pos,sf::Vector2f(),sf::Vector2f(coordsys.x_basis())))
			new_title+="cursor on left of x axis; ";
		else new_title+="cursor on right of x axis; ";
		if(mmoc::point_on_left(mouse_pos,sf::Vector2f(),sf::Vector2f(coordsys.y_basis())))
			new_title+="cursor on left of y axis";
		else new_title+="cursor on right of y axis";
		if(mmoc::point_on_left(sf::Vector2f(2.f,1.f),sf::Vector2f(),sf::Vector2f(1.f,1.f)))
			new_title+="; true";
		else new_title+="; false";
		if(!mmoc::point_on_left(sf::Vector2f(1.f,2.f),sf::Vector2f(),sf::Vector2f(1.f,1.f)))
			new_title+="; true";
		else new_title+="; false";
		window.setTitle(new_title);
		
		//y basis
		mmoc::CoordSys2D<double> x_basis_is_y_basis(coordsys.y_basis());
		
		//transformed
		point.setFillColor(sf::Color(127,255,127));
		point.setPosition(sf::Vector2f(vec*x_basis_is_y_basis));
		draw_point();
		//transformed twice
		point.setPosition(sf::Vector2f(vec*x_basis_is_y_basis*x_basis_is_y_basis));
		draw_point();
		//inverse transformed
		point.setFillColor(sf::Color(0,127,0));
		point.setPosition(sf::Vector2f(vec/x_basis_is_y_basis));
		draw_point();
		//inverse transformed twice
		point.setPosition(sf::Vector2f(vec/x_basis_is_y_basis/x_basis_is_y_basis));
		draw_point();
		
		//x basis
		
		//transformed
		point.setFillColor(sf::Color(255,127,127));
		point.setPosition(sf::Vector2f(coordsys*vec));
		draw_point();
		//transformed twice
		point.setPosition(sf::Vector2f(vec*(coordsys*coordsys)));
		draw_point();
		//inverse transformed
		point.setFillColor(sf::Color(127,0,0));
		point.setPosition(sf::Vector2f(vec/coordsys));
		draw_point();
		//inverse transformed twice
		point.setPosition(sf::Vector2f(vec/coordsys*(coordsys/coordsys/coordsys)));
		draw_point();
		
		//vector
		point.setFillColor(sf::Color(127,127,127));
		point.setPosition(sf::Vector2f(vec));
		draw_point();
		
		//x and y basis
		point.setFillColor(sf::Color::Red);
		point.setPosition(sf::Vector2f(coordsys.x_basis()));
		draw_point();
		
		point.setFillColor(sf::Color::Green);
		point.setPosition(-point.getPosition().y,point.getPosition().x);
		draw_point();
		
		//drawing [1,0] and [0,0] points for reference
		point.setFillColor(sf::Color::Cyan);
		point.setPosition(1.f,0.f);
		draw_point();
		
		point.setFillColor(sf::Color::Magenta);
		point.setPosition(0.f,0.f);
		draw_point();
		
		//finish
		window.display();
		
		sf::sleep(sf::seconds(1.f/fps)-clock.restart());
	}
}
