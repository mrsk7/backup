import Data.Char
import System.IO
import qualified Data.Map as Map
import Data.Maybe
import Debug.Trace as Trace
import Data.Set as Set

data Type  =  Tvar Int | Tfun Type Type                        deriving Eq
data Expr  =  Evar String | Eabs String Expr | Eapp Expr Expr  deriving Eq
data Aexpr = Avar String Type | Aabs String Aexpr Type | Aapp Aexpr Aexpr Type deriving (Eq,Show)

type Substitution = [(Int,Type)]

-- Pretty printing of expressions

always = True    -- False omits parentheses whenever possible

instance Show Expr where
  showsPrec p (Evar x) = (x ++)
  showsPrec p (Eabs x e) =
    showParen (always || p > 0) ((("\\" ++ x ++ ". ") ++) . showsPrec 0 e)
  showsPrec p (Eapp e1 e2) =
    showParen (always || p > 1) (showsPrec 1 e1 . (" " ++) . showsPrec 2 e2)

-- Parsing of expressions

instance Read Expr where
  readsPrec _ s =
    readParen True (\s ->
      [(Eabs x e, r)    |  ("\\", t1)  <-  lex s,
                           (x, t2)     <-  lex t1, isVar x,
                           (".", t3)   <-  lex t2,
                           (e, r)      <-  readsPrec 0 t3] ++
      [(Eapp e1 e2, r)  |  (e1, t)     <-  readsPrec 0 s,
                           (e2, r)     <-  readsPrec 0 t]) s ++
    [(Evar x, r) | (x, r) <- lex s, isVar x]
      where isVar x = isAlpha (head x) && all isAlphaNum x

-- Pretty printing of types

instance Show Type where
  showsPrec p (Tvar alpha) = ("@" ++) . showsPrec 0 alpha
  showsPrec p (Tfun sigma tau) =
    showParen (p > 0) (showsPrec 1 sigma . (" -> " ++) . showsPrec 0 tau)

type_of :: Aexpr -> Type
type_of (Avar _ t) = t
type_of (Aabs _ _ t) = t
type_of (Aapp _ _ t) = t

-- Creates new type variables for each expression, maps expressions to types and returns and Aexpr and the next typevariable to be assigned
setTypeVar :: Int -> Expr -> Map.Map String Int -> (Aexpr,Int,(Map.Map String Int))
setTypeVar tv (Evar x) mapEtoT =
   let   
         mb = Map.lookup x mapEtoT
         mapEtoTupdated = if (isNothing mb) then Map.insert x tv mapEtoT else mapEtoT
         typ = maybe tv id mb			-- If term unbound then type is tv else type is type of found term
         tv_next = if (isNothing mb) then tv+1 else tv
   in    ((Avar x (Tvar typ)),tv_next,mapEtoTupdated)
setTypeVar tv (Eabs x e) mapEtoT =
   let   mapEtoTupdated = Map.insert x tv mapEtoT
         (ae,ntv,mapfinal) = setTypeVar (tv+1) e mapEtoTupdated
   in    ((Aabs x ae (Tfun (Tvar tv) (type_of ae))),ntv,mapfinal)
setTypeVar tv (Eapp e1 e2) mapEtoT =
   let   (ae1,tv1,map1) = setTypeVar tv e1 mapEtoT 
         (ae2,tv2,map2) = setTypeVar tv1 e2 map1 
   in    ((Aapp ae1 ae2 (Tvar tv2)),tv2+1,map2)

collect' :: Aexpr -> [Aexpr] -> [(Type,Type)] -> [(Type,Type)]
collect' (Avar _ _) tail l = collect tail l
collect' (Aabs _ ae _ ) tail l = collect ([ae] ++ tail) l
collect'  (Aapp ae1 ae2 a) tail l=
   let  (t1,t2) = (type_of ae1, type_of ae2)
   in   collect ([ae1] ++ [ae2] ++ tail) ([(t1,(Tfun t2 a))] ++ l)

collect :: [Aexpr] -> [(Type,Type)] -> [(Type,Type)]
collect [] l = l
collect (head:tail) l = collect' head tail l

occurs :: Int -> Type -> Bool
occurs x (Tvar a) = (x==a)
occurs x (Tfun a b) = occurs x a || occurs x b


--Substitute type t1 for all var str in type t2
substitute :: Int -> Type -> Type -> Type
substitute var t1 t2@(Tvar x) = if (var==x) then t1 else t2
substitute var t1 t2@(Tfun x y) = (Tfun (substitute var t1 x) (substitute var t1 y))

apply :: Maybe Substitution -> Type -> Type
apply (Just sub) typ = Prelude.foldr (\(x,y) -> substitute x y) typ sub 

unify' :: Type -> Type -> Maybe Substitution
unify' (Tvar x) t1@(Tvar y)  = if (x==y) then Just [] else Just [(x,t1)]
unify' (Tfun x y) (Tfun z w) =  unify [(x,z),(y,w)]
unify' (Tvar x) t2@(Tfun z w) = if (occurs x t2) then Nothing else Just [(x,t2)]
unify' t3@(Tfun x y) (Tvar z) = if (occurs z t3) then Nothing else Just [(z,t3)]
           
unify :: [(Type,Type)] -> Maybe Substitution
unify [] = Just []
unify ((a,b):ts) =
   let  t1 =  unify ts
        t2 = unify' (apply t1 a) (apply t1 b)
   in if ((isJust t1) && (isJust t2)) then Just ((fromJust t2) ++ (fromJust t1)) else Nothing

getUsedVariables :: Type -> Set.Set Int -> Set.Set Int
getUsedVariables (Tvar x) set = Set.insert x set
getUsedVariables (Tfun a b) set = let s1 = getUsedVariables a set
                                  in  getUsedVariables b s1

renamingConstraints :: (Int,Set.Set Int,Substitution) -> (Int,Set.Set Int,Substitution)
renamingConstraints (c,set,sub) = if (Set.null set)
                                  then (c,set,sub)
                                  else let (old,newset) = Set.deleteFindMin set
                                           new =  c
                                           newsub = [(old,Tvar new)] ++ sub
                                       in renamingConstraints (c+1,newset,newsub)

readOne  =  do  s <- getLine
                let e = read s :: Expr
                let newMap = Map.empty
                let (aexp,b,c) = setTypeVar 0 e newMap
                let constraints = collect [aexp] []
                let subt = unify constraints
                if (isNothing subt) then putStrLn "type error"
                else 
                     let  tmp = apply subt (type_of aexp)
                          set = Set.empty
                          uv = getUsedVariables tmp set
                          (_,_,subt') = renamingConstraints (0,uv,(fromJust subt))
                          final = apply (Just subt') (type_of aexp)
                     in putStrLn $ show final

count n m  =  sequence $ take n $ repeat m

main     =  do  n <- readLn
                count n readOne
