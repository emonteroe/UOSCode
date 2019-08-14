%s/plot.*//g
%g/set t.*/d
%s/^e.*//g
%g/set y.*/d
%g/set x.*/d
%g/^$/d
%s/set.*\([0-9]\).*/\rRUN \1/g

