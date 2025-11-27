program erro_lexico;
var
  x : real;
begin
  x := 10.5;
  { O caractere @ nao existe no MicroPascal }
  x := x @ 2; 
  
  { String nao fechada tambem deve quebrar }
  x := 'string sem fim...
end.