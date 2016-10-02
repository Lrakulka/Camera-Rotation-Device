


$(document).ready(function(){
  

  
  /**
   * Fast Navigator
   */

   
  var navigator_category = $.jStorage.get('last_navigator_category')	
  var navigator_hubs = $.jStorage.get('last_navigator_hubs')
  var navigator_hub = $.jStorage.get('last_navigator_hub')

  if( navigator_category && navigator_category != 'null') {
  	$('#fast_navagator select[name="category"]').val(navigator_category)
    //console.log(111)
    if( navigator_hubs ) {
      var hubs = '';
      for(k in navigator_hubs){
        hubs += '<option value="'+navigator_hubs[k].alias+'">'+navigator_hubs[k].name+'</option>';
      }
      $('#fast_navagator select[name="hub"]').html(hubs).attr('disabled', false);
      $('#fast_navagator input[type="submit"]').attr('disabled', false);
    }
    if( navigator_hub ) {
      $('#fast_navagator select[name="hub"]').val(navigator_hub);
      $('#fast_navagator .buttons input').attr('disabled',false).removeClass('loading')
    }
  }else{
    $('#fast_navagator select[name="hub"]').html('<option>Хаб</option>').attr('disabled', true);
    $('#fast_navagator input[type="submit"]').attr('disabled', true);
  }

  $('#fast_navagator select[name="category"]').change(function(){
    var alias = $(this).val();
    
    //console.log( alias )
    $.jStorage.set('last_navigator_category', alias)
    
    if(alias == 'null'){
    	$('#fast_navagator select[name="hub"]').html('<option>Хаб</option>').attr('disabled', true);
      $('#fast_navagator input[type="submit"]').attr('disabled', true);
    }else{
	    $.post('/json/hubs/category/'+alias+'/', function(json){
	      if(json.messages=='ok'){
	        var hubs = '';
	        for(k in json.hubs){
	          hubs += '<option value="'+json.hubs[k].alias+'">'+json.hubs[k].name+'</option>';
	        }
	        $.jStorage.set('last_navigator_hubs', json.hubs);
	        $('#fast_navagator select[name="hub"]').html(hubs).attr('disabled', false);
	        $('#fast_navagator input[type="submit"]').attr('disabled', false);
	      }else{
	        show_system_error(json);
	      }
	    },'json');   
    }
  });
  
  $('#fast_navagator').submit(function(){
    var hub = $('#fast_navagator select[name="hub"]').val();
    $.jStorage.set('last_navigator_hub', hub)
    document.location.href = '/hub/'+hub+'/';
    return false;
  });
  
  
  
  
  
  
  /** 
   * Нравится/не нравится компания
   */
  $('#addCompanyFan').click(function(){
    var id = $(this).attr('data-id');
    var link = $(this);
  			link.addClass('loading');
    $.post('/json/corporation/fan_add/', {'company_id':id}, function(json){
      if(json.messages =='ok'){
        $('#removeCompanyFan').removeClass('hidden');
        $('#addCompanyFan').addClass('hidden'); 
        $('#fans_count').text(json.fans_count_str);       
      }else{
        show_system_error(json);
      }
      link.removeClass('loading');
    },'json');
    return false;
  });
  $('#removeCompanyFan').click(function(){
    var id = $(this).attr('data-id');
		var link = $(this);
  			link.addClass('loading');
      $.post('/json/corporation/fan_del/', {'company_id':id}, function(json){
        if(json.messages =='ok'){
          $('#removeCompanyFan').addClass('hidden');
          $('#addCompanyFan').removeClass('hidden');        
          $('#fans_count').text(json.fans_count_str);
        }else{
          show_system_error(json);
        }
        link.removeClass('loading');

      },'json');

    return false;
  });
  
  
  
  


});