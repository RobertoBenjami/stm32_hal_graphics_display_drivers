#include "Fonts\fonts.h"

const uint8_t led_48x48_Table[] =
{
  // @0 ' ' (48 pixels wide)
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x0F,0x00,0x00,0x00, //                     ####                        
  0x00,0x01,0xFF,0xFC,0x00,0x00, //                ###############                  
  0x00,0x07,0xFF,0xFF,0x00,0x00, //              ###################                
  0x00,0x0F,0xFF,0xFF,0x80,0x00, //             #####################               
  0x00,0x1F,0xFF,0xFF,0xC0,0x00, //            #######################              
  0x00,0x7F,0xFF,0xFF,0xF0,0x00, //          ###########################            
  0x00,0xFF,0xFF,0xFF,0xF8,0x00, //         #############################           
  0x00,0xFF,0xFF,0xFF,0xF8,0x00, //         #############################           
  0x01,0xFF,0xFF,0xFF,0xFC,0x00, //        ###############################          
  0x03,0xFF,0xFF,0xFF,0xFE,0x00, //       #################################         
  0x03,0xDF,0xFF,0xFF,0xFE,0x00, //       #### ############################         
  0x07,0xDF,0xFF,0xFF,0xFF,0x00, //      ##### #############################        
  0x0F,0xBF,0xFF,0xFF,0xFF,0x80, //     ##### ###############################       
  0x0F,0x7F,0xFF,0xFF,0xFF,0x80, //     #### ################################       
  0x0F,0x7F,0xFF,0xFF,0xFF,0x80, //     #### ################################       
  0x0F,0x7F,0xFF,0xFF,0xFF,0x80, //     #### ################################       
  0x0F,0x7F,0xFF,0xFF,0xFF,0x80, //     #### ################################       
  0x1F,0xFF,0xFF,0xFF,0xFF,0xC0, //    #######################################      
  0x1E,0xFF,0xFF,0xFF,0xFF,0xC0, //    #### ##################################      
  0x1E,0xFF,0xFF,0xFF,0xFF,0xC0, //    #### ##################################      
  0x1E,0xFF,0xFF,0xFF,0xFF,0xC0, //    #### ##################################      
  0x1E,0xFF,0xFF,0xFF,0xFF,0xC0, //    #### ##################################      
  0x0E,0xFF,0xFF,0xFF,0xFF,0x80, //     ### #################################       
  0x0F,0xFF,0xFF,0xFF,0xFF,0x80, //     #####################################       
  0x0F,0x7F,0xFF,0xFF,0xFF,0x80, //     #### ################################       
  0x0F,0x7F,0xFF,0xFF,0xFF,0x80, //     #### ################################       
  0x0F,0x7F,0xFF,0xFF,0xFF,0x80, //     #### ################################       
  0x0F,0xFF,0xFF,0xFF,0xFF,0x80, //     #####################################       
  0x07,0xFF,0xFF,0xFF,0xFF,0x00, //      ###################################        
  0x03,0xDF,0xFF,0xFF,0xFE,0x00, //       #### ############################         
  0x03,0xFF,0xFF,0xFF,0xFE,0x00, //       #################################         
  0x01,0xFF,0xFF,0xFF,0xFC,0x00, //        ###############################          
  0x00,0xFF,0xFF,0xFF,0xF8,0x00, //         #############################           
  0x00,0xFF,0xFF,0xFF,0xF8,0x00, //         #############################           
  0x00,0x7F,0xFF,0xFF,0xF0,0x00, //          ###########################            
  0x00,0x1F,0xFF,0xFF,0xC0,0x00, //            #######################              
  0x00,0x0F,0xFF,0xFF,0x80,0x00, //             #####################               
  0x00,0x07,0xFF,0xFF,0x00,0x00, //              ###################                
  0x00,0x01,0xFF,0xFC,0x00,0x00, //                ###############                  
  0x00,0x00,0x0F,0x00,0x00,0x00, //                     ####                        
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 

  // @288 '!' (48 pixels wide)
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x10,0x00,0x00, //                            #                    
  0x00,0x00,0x00,0x38,0x00,0x00, //                           ###                   
  0x00,0x00,0x00,0x3C,0x00,0x00, //                           ####                  
  0x00,0x00,0x00,0x3E,0x00,0x00, //                           #####                 
  0x00,0x00,0x00,0x1F,0x00,0x00, //                            #####                
  0x00,0x00,0x00,0x0F,0x80,0x00, //                             #####               
  0x00,0x00,0x00,0x0F,0x80,0x00, //                             #####               
  0x00,0x00,0x00,0x07,0xC0,0x00, //                              #####              
  0x00,0x00,0x00,0x07,0xC0,0x00, //                              #####              
  0x00,0x00,0x00,0x07,0xC0,0x00, //                              #####              
  0x00,0x00,0x00,0x07,0xE0,0x00, //                              ######             
  0x00,0x00,0x00,0x07,0xE0,0x00, //                              ######             
  0x00,0x00,0x00,0x07,0xE0,0x00, //                              ######             
  0x00,0x00,0x00,0x07,0xE0,0x00, //                              ######             
  0x00,0x00,0x00,0x07,0xE0,0x00, //                              ######             
  0x00,0x00,0x00,0x07,0xE0,0x00, //                              ######             
  0x00,0x00,0x00,0x07,0xC0,0x00, //                              #####              
  0x00,0x00,0x00,0x07,0xC0,0x00, //                              #####              
  0x00,0x00,0x00,0x0F,0xC0,0x00, //                             ######              
  0x00,0x00,0x00,0x1F,0x80,0x00, //                            ######               
  0x00,0x00,0x00,0x3F,0x00,0x00, //                           ######                
  0x00,0x00,0x00,0x7F,0x00,0x00, //                          #######                
  0x00,0x00,0x00,0x7E,0x00,0x00, //                          ######                 
  0x00,0x00,0x00,0x7C,0x00,0x00, //                          #####                  
  0x00,0x00,0x00,0xB8,0x00,0x00, //                         # ###                   
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
  0x00,0x00,0x00,0x00,0x00,0x00, //                                                 
};

sFONT led_48x48 =
{
  led_48x48_Table,
  48, /* Width */
  48, /* Height */
};
