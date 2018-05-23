# DirectX 11 Application

[YouTube Video](https://www.youtube.com/watch?v=K13M7ugoTIU)

### List of shaders  
 * ColourShader - renders a texture *colour_vs*, *colour_ps*  
 * DepthOfField - Applies​ ​all​ ​final​ ​post​ ​processing​ ​effects​ ​- *DOF_vs*, *DOF_ps* 
 * BlurShader - gaussian​ ​blur​ ​-*blur_vs*, *blur_ps*  
 * DepthShader - renders​ ​depth​ ​of​ ​the​ ​scene​ ​in​ ​red​ ​channel​ ​and​ ​focus​ ​point​ ​for​ ​depth  of​ ​field​ ​in​ ​green​ ​channel​ ​-*depth_vs*, *depth_ps*  
 * DepthTesselationShader - same​ ​as​ ​DepthShader​ ​but​ ​is​ ​also​ ​applying​ ​tesselation​ ​so  depth​ ​can​ ​be​ ​calculated​ ​correctly​ ​also​ ​for​ ​tesselated​ ​objects​ ​- *displacement_vs*, *dispalcement_hs*, *depthTesselation_ds*, *depthTesselation_ps*  
 * DisplacementShader - applies​ ​displacement​ ​map​ ​for​ ​tesselation​ ​- *displacement_vs*, *dispalcement_hs*, *displacement_ds*, *object_ps* 
 * GeometryShader - grass​ ​generation​ ​- *geometry_vs*, *geometry_gs*, *geometry_ps*  
 * ObjectShader - all​ ​interesting​ ​pixel​ ​shader​ ​bits​ ​are​ ​calculated​ ​here​ ​- *object_vs*,  *object_ps*  
 * SkyboxShader - same​ ​as​ ​ColourShader​ ​but​ ​sampling​ ​a​ ​cubemap  

### Rendering
 1. Render​ ​shadow​ ​maps  
    1. Loop through all​ ​enabled​ ​lights  
 2. Render​ ​depth​ ​to​ ​texture(from​ ​camera​ ​point​ ​of​ ​view)  
 3. Render​ ​scene​ ​to​ ​texture(to​ ​apply​ ​post​ ​processing​ ​effects later on)  
     1. Generate​ ​cubemap​ ​reflections​ ​if​ ​any​ ​object​ ​needs​ ​them  
     2. Render​ ​the​ ​scene  
 4. Render​ ​post​ ​processing​ ​to​ ​texture  
     1. Blur​ ​down​ ​sampled​ ​texture(Gaussian)  
     2. Upsample​ ​the​ ​texture  
     3. Apply​ ​post​ ​processing​ ​effects(Shader​ ​is​ ​called​ ​DOF​ ​as​ ​I​ ​forgot​ ​to​ ​change​ ​the  name​ ​but​ ​besides​ ​depth​ ​of​ ​field​ ​it’s​ ​also​ ​applying​ ​vignette​ ​and​ ​black/white  colour)  
 5. Render the scene​ ​on​ ​ortho​ ​plane 
