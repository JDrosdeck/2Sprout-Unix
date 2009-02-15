Shoes.app :title => "2sprout Manager", :width => 320, :height => 420, :resizable => false do
  background "../static/menu-gray.png", :radius => 12
  background "../static/menu-top.png", :height => 50
  background "../static/menu-left.png", :top => 50, :width => 55
  background "../static/menu-right.png", :right => 0, :top => 50, :width => 55
  image "../static/menu-corner1.png", :top => 0, :left => 0
  image "2sprout.png", :right => 0, :top => 215

  @firstSet = stack :margin => 60 do
    stack :margin => 5 do
      para "User Name"
      @userName = edit_line
    end
    stack :margin => 10 do
      para "Password"
      @pass = edit_line
    end
    stack :margin => 10 do
       @button = button "Login" do
        Shoes.p [@userName.text, @pass.text]
      end
    end
    
    @button.click do
        if (@userName.text != '' and @pass.text != '') then
    	@firstSet.clear
    	#start loading bar
    	stack :margin => 50 do
    	para "Connecting to 2sprout"
    	@prog = progress :width => 1.0
    	animate do |i|
    	@prog.fraction = (i %100 ) / 100.0
    	end
    	end
    	
   
    	end 
    
    end
    
    
  end
end



