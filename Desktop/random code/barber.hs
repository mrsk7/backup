shaves :: Int -> Int -> Bool
shaves 1 1 = True
shaves 2 2 = False
shaves 0 x = not (shaves x x)
shaves _ _ = False
