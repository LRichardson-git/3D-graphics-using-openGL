#version 330




//in vec2 UV;

//out vec3 color;

//uniform sampler2D renderedTexture;
//uniform float time;

out vec3 fragment_colour;

in vec2 UV;

//out vec3 color;

uniform sampler2D Texture;
uniform float time;
vec4 colourT;




//turn into uniforms
float FXAA_Threshhold_min;
float FXAA_Treshhold_max;



//caluate the luminoisty of the pixels

//perceptual values

float LumaRGB(vec3 rgb){
	return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));
}



//used to sample pixels and see the weighted sum of RGB components and take into account sensibility of our eyes to each wavelength range


void main(){
   




   





   //value thresholds for lumonsity 
   FXAA_Threshhold_min = 0.0312f;
   FXAA_Treshhold_max = 0.125f;

   //change to unifroms
   float invScreenSizeX = 1/1280f;
  float invscreensizeY =  1/720f;


   vec3 Colours = texture(Texture, UV).rbg;

   //The lumonisity at the current fragment
   float CenterLuma = LumaRGB(Colours);

   //The lumonisty at the four neighbouts of current frag
   float DownLuma = LumaRGB(textureOffset(Texture, UV, ivec2(0,-1)).rgb);
   float UpLuma = LumaRGB(textureOffset(Texture, UV, ivec2(0,1)).rgb);
   float LeftLuma = LumaRGB(textureOffset(Texture, UV, ivec2(-1,0)).rgb);
   float RightLuma = LumaRGB(textureOffset(Texture, UV, ivec2(1,0)).rgb);

   //max luma and min lim around the current frag
   float MinLuma = min (CenterLuma, min(min(DownLuma,UpLuma),min(LeftLuma,RightLuma)));
   float MaxLuma = max (CenterLuma, max(max(DownLuma,UpLuma),max(LeftLuma,RightLuma)));

   //get the delta
   float RangeLuma = MaxLuma - MinLuma;

   //if dark area, on edge or below theshold then dont DO no do any anti alaising and return the texture as is
   if(RangeLuma < max(FXAA_Threshhold_min, MaxLuma * FXAA_Treshhold_max)) {

	 fragment_colour = texture(Texture, UV).rgb;

	 return;
   }

   
   // do the corners now (lumonsity)
	float DownLeftLuma = LumaRGB(textureOffset(Texture,UV,ivec2(-1,-1)).rgb);
	float UpRightLuma = LumaRGB(textureOffset(Texture,UV,ivec2(1,1)).rgb);
	float UpLeftLuma = LumaRGB(textureOffset(Texture,UV,ivec2(-1,1)).rgb);
	float DownRightLuma = LumaRGB(textureOffset(Texture,UV,ivec2(1,-1)).rgb);

	

	//combine corners
	float LCornerLuma = DownLeftLuma + UpLeftLuma;
	float DCornerLuma = DownLeftLuma + DownRightLuma;
	float RCornerLuma = DownRightLuma + UpRightLuma;
	float UpCornerLuma = UpRightLuma + UpLeftLuma;


	//combine edges
	float DWLuma = DownLuma + UpLuma;
	float LRLuma = LeftLuma + RightLuma;


	//estimate graidennt along horizontal and vertical axis
	float VerticalEdge = abs(-2.0 * UpLuma + UpCornerLuma) + abs(-2.0 * CenterLuma + LRLuma) * 2.0  + abs(-2.0 * DownLuma + DCornerLuma);
	float HorizontEdge = abs(-2.0 * LeftLuma + LCornerLuma)  + abs(-2.0 * CenterLuma + DWLuma ) * 2.0    + abs(-2.0 * RightLuma + RCornerLuma);
	
	bool horizontal = (HorizontEdge >= VerticalEdge);
   

   //edge oriention

   // neighbouting in opp edge
	float FirstLuma = horizontal ? DownLuma : LeftLuma;
	float SecdLuma = horizontal ? UpLuma : RightLuma;

	// Gradients
	float FirstGradient = FirstLuma - CenterLuma;
	float SecdGradient = SecdLuma - CenterLuma;

	// normalized gradient
	float gradientScaled = 0.25*max(abs(FirstGradient),abs(SecdGradient));

	// find steepetst direction
	bool is1Steepest = abs(FirstGradient) >= abs(SecdGradient);

	//gradient callaculated scaled 


	//move half pixel in the direction
	//aclualte the average luma ththis point


	// Choose the step size 
	float stepLength = horizontal ? invscreensizeY : invScreenSizeX;

	// Average luma 
	float LocalAverageLuma = 0.0;

	if(is1Steepest){
		// Switch the direction
		stepLength = - stepLength;
		LocalAverageLuma = 0.5*(FirstLuma + CenterLuma);
	} else {
		LocalAverageLuma = 0.5*(SecdLuma + CenterLuma);
	}

	// Shift UV // half pixel
	vec2 currUv = UV;
	if(horizontal){
		currUv.y += stepLength * 0.5;
	} else {
		currUv.x += stepLength * 0.5;
	}



	vec2 offset = horizontal ? vec2(invScreenSizeX,0.0) : vec2(0.0,invscreensizeY);

	vec2 uv1 = currUv - offset;
	vec2 uv2 = currUv + offset;

	float End1 = LumaRGB(texture(Texture,uv1).rgb);
	float End2 = LumaRGB(texture(Texture,uv2).rgb);

	End1 -= LocalAverageLuma;
	End2 -= LocalAverageLuma;


	bool reached1 = abs(End1) >= gradientScaled;
	bool reached2 = abs(End2) >= gradientScaled;
	bool reachedBoth = reached1 && reached2;


	if(!reached1){
		uv1 -= offset;
	}

	if(!reached2){
		uv2 += offset;
	}   


	float Iterations_amount = 15; 
	float Search_Quality [15];
	
	for(int i = 0; i  < 5; i++){

	Search_Quality[i] = 1;
	};

	Search_Quality[5] = 1.5f;
	Search_Quality[6] = 2f;
	Search_Quality[7] = 2f;
	Search_Quality[8] = 2f;
	Search_Quality[9] = 2f;
	Search_Quality[10] = 4f;
	Search_Quality[11] = 8f;
	Search_Quality[12] = 9f;
	Search_Quality[13] = 10f;
	Search_Quality[14] = 12f;


	// check if sides have been reached
	if(!reachedBoth){

		for(int i = 2; i < Iterations_amount; i++){
			// if, get delta
			if(!reached1){
				End1 = LumaRGB(texture(Texture, uv1).rgb);
				End1 = End1 - LocalAverageLuma;
			}
			// read luma, opposiste direction
			if(!reached2){
				End2 = LumaRGB(texture(Texture, uv2).rgb);
				End2 = End2 - LocalAverageLuma;
			}
			//if larger than gradient
			reached1 = abs(End1) >= gradientScaled;
			reached2 = abs(End2) >= gradientScaled;
			reachedBoth = reached1 && reached2;

			
			if(!reached1){
				uv1 -= offset * Search_Quality[i];
			}

			if(!reached2){
				uv2 += offset * Search_Quality[i];
			}

			
			if(reachedBoth){ break;}
		}
	}



	//ESTIMAate offset


		
	float FirstDist = horizontal ? (UV.x - uv1.x) : (UV.y - uv1.y);
	float SecdDist = horizontal ? (uv2.x - UV.x) : (uv2.y - UV.y);

	
	bool isFirstDirection = FirstDist < SecdDist;
	float distanceFinal = min(FirstDist, SecdDist);

	float ThicknessEdge = (FirstDist + SecdDist);

	// UV offset: read in the direction of the closest side of the edge.
	float pixelOffset = - distanceFinal / ThicknessEdge + 0.5;
	bool isLumaCenterSmaller = CenterLuma < LocalAverageLuma;

	// If the luma at center is smaller than at its neighbour, the delta luma at each end should be positive (same variation).
	// (in the direction of the closer side of the edge.)
	bool Varition = ((isFirstDirection ? End1 : End2) < 0.0) != isLumaCenterSmaller;
	float finalOffset = Varition ? pixelOffset : 0.0;

	//Sub pixel
	float Quality_Pixel = 0.75;
		
	// Weighted average of luma , ratio of delta in 3x3 and delta of offset
	float lumaAverage = (1.0/12.0) * (2.0 * (DWLuma + LRLuma) + LCornerLuma + RCornerLuma);
	float Pixel_Offest_1 = clamp(abs(lumaAverage - CenterLuma)/RangeLuma,0.0,1.0);
	float Pixel_Offest_2 = (-2.0 * Pixel_Offest_1 + 3.0) * Pixel_Offest_1 * Pixel_Offest_1;
	float PixelOffsetFinal = Pixel_Offest_2 * Pixel_Offest_2 *  Quality_Pixel;

	// larger offset
	finalOffset = max(finalOffset,PixelOffsetFinal);



	// Get the final Coords
	vec2 FinalTexCoordsUV = UV;
	if(horizontal){
		FinalTexCoordsUV.y += finalOffset * stepLength;
	} else {
		FinalTexCoordsUV.x += finalOffset * stepLength;
	}

	// Set the colour to the correct coords
	vec3 finalColor = texture(Texture,FinalTexCoordsUV).rgb;
	fragment_colour = finalColor;


}