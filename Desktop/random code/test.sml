fun all_picks [] = []
|   all_picks lst = 
        let
            fun help [f] left acc = acc@[(f,left)]
            |   help (f::rest) left acc = 
                help rest (left@[f]) acc@[(f,left@rest)]
        in
            help lst [] []
end
