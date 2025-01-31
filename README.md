Recent Update: Redid movement (not finished) and way the player is handeled, will be refined after actually organising my code.

Need to Do list:

---Priority---
- Need to work on redoing everything seperating it all into different files, this could help efficiency and make it easier to read, will also help with the fact that the code is getting very long.
- Need to refine the shaders (if i am keeping them) as the way they are loaded and managed currently is not efficient.
- File manager, for simplicity of being able to find parts of code easier, as in files for shaders etc etc.

-- Maps could be done by having a map loader that loads in the objects and textures, the map loader would be built from config files that would be loaded in at the start of the game, menu would pick between them.


- Remember to Update the README everytime i push a new change.
---Secondary---
- FPS limiter, need to add one as i can hear my fans crying every time i run this. (Maybe find a way to load faster(Or it could be just how DX11 loads?))
- Fix Collision as the player can still currently warp up through the floor.

- Need to think more about how the game will work, while the baseline is okay need more idea on it like multiple guns? More maps? Enemines? and general feeling for the game.
-- Am liking the idea of a base parkour game for the beginning, can add multiple guns with a switch statment to change the way they fire,(And maybe their model) Enemies would be a long way off if not impossible because of the way the game is designed. 

-- Have Implemented new movement as a way around, this movement is only controlled by the W key and the player moving the mouse the player loses controller of the W key while in air and has to reply on the launching to get to the end.
- Add in a way to let the player have in air controls to make teh movement while falling feel smoother, can also change gravity if needed as it right now is strange.
- change the wya the shaders are loaded should be able to be done in a loop.
- Change the way they are added to an object should be able to do that inside of where we create the object.