// Preload images for makes.
if (document.images) 
{
    var aMakeImage = new Array(32);
   
    aMakeImage[0] = 'CE_Images/CAR_File.png';
    aMakeImage[1] = 'CE_Images/Makes/Audi.jpg';
    aMakeImage[2] = 'CE_Images/Makes/BMW.jpg';
    aMakeImage[3] = 'CE_Images/Makes/Cadillac.jpg';
    aMakeImage[4] = 'CE_Images/Makes/Chevrolet.jpg';
    aMakeImage[5] = 'CE_Images/Makes/Chrysler.jpg';
    aMakeImage[6] = 'CE_Images/Makes/Ferrari.jpg';
    aMakeImage[7] = 'CE_Images/Makes/Ford.jpg';
    aMakeImage[8] = 'CE_Images/Makes/GMC.jpg';
    aMakeImage[9] = 'CE_Images/Makes/Honda.jpg';
    aMakeImage[10] = 'CE_Images/Makes/Hyundai.jpg';
    aMakeImage[11] = 'CE_Images/Makes/Infiniti.jpg';
    aMakeImage[12] = 'CE_Images/Makes/Jaguar.jpg';
    aMakeImage[13] = 'CE_Images/Makes/Jeep.jpg';
    aMakeImage[14] = 'CE_Images/Makes/Kia.jpg';
    aMakeImage[15] = 'CE_Images/Makes/Lamborghini.jpg';
    aMakeImage[16] = 'CE_Images/Makes/Land_Rover.jpg';
    aMakeImage[17] = 'CE_Images/Makes/Mazda.jpg';
    aMakeImage[18] = 'CE_Images/Makes/Mazerati.jpg';
    aMakeImage[19] = 'CE_Images/Makes/Mercedes.jpg';
    aMakeImage[20] = 'CE_Images/Makes/MINI.jpg';
    aMakeImage[21] = 'CE_Images/Makes/Mitsubishi.jpg';
    aMakeImage[22] = 'CE_Images/Makes/Nissan.jpg';
    aMakeImage[23] = 'CE_Images/Makes/Peugeot.jpg';
    aMakeImage[24] = 'CE_Images/Makes/Porsche.jpg';
    aMakeImage[25] = 'CE_Images/Makes/Renault.jpg';
    aMakeImage[26] = 'CE_Images/Makes/Saab.jpg';
    aMakeImage[27] = 'CE_Images/Makes/Smart.jpg';
    aMakeImage[28] = 'CE_Images/Makes/Subaru.jpg';
    aMakeImage[29] = 'CE_Images/Makes/Suzuki.jpg';
    aMakeImage[30] = 'CE_Images/Makes/Toyota.jpg';
    aMakeImage[31] = 'CE_Images/Makes/Volkswagen.jpg';

    // Start preloading.
    for (i=0; i < 32; i++) 
    {
        var objImage = new Image();
        objImage.src = aMakeImage[i];
    }  
}