/*  源文件“main.cpp”*/
　int main()
　{
 　 Circle circle;
 　 circle.SetOrigin( 0.0, 0.0 );
 　 circle.SetRadius( 1.0 );
　  cout << "Perimeter: " << circle.GetPerimeter() << endl;
　  cout << "Area: " << circle.GetArea() << endl;
　  return 0;
　};