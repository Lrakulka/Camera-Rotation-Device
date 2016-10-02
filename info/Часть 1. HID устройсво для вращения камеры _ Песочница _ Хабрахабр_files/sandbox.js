$(document).ready(function(){

	// подарить приглашение
	$('.post .infopanel input[name="send_invite"]').live('click', function(){
		var id = $(this).data('id');
		$.post('/json/users/give_invite_bypost/', { post_id: id }, function(json){
			if(json.messages=='ok'){
				$('#infopanel_post_'+id+' .send_invite_wrapper').html('<span style="color:green;">Приглашение отправлено</span>');
			}else{
				show_system_error(json);
			}
		},'json');
		return false;
	});

    // подарить приглашение
    $('.post .infopanel input[name="send_invite_plus"]').live('click', function(){
        var id = $(this).data('id');
        $.post('/json/users/give_invite_bypost_plus/', { post_id: id }, function(json){
            if(json.messages=='ok'){
                $('#infopanel_post_'+id+' .send_invite_wrapper').html('<span style="color:green;">Приглашение отправлено</span>');
            }else{
                show_system_error(json);
            }
        },'json');
        return false;
    });


	// удалить
	$('.post .infopanel input[name="delete"]').live('click', function(){
		var id = $(this).data('id');
		$.post('/json/sandbox/delete/', { post_id: id }, function(json){
			if(json.messages=='ok'){
				$('#post_'+id).fadeOut(500,function(){
					$(this).remove();
				});
			}else{
				show_system_error(json);
			}
		},'json');
		return false;
	});

    // удалить
    $('.post .infopanel input[name="reject"]').live('click', function(){
      var id = $(this).data('id');
      $('#reject_form_post_'+id).removeClass('hidden');
      $('#transfer_form_post_'+id).addClass('hidden');

      var rejectSelectList = $('#reject_form_post_'+id+' select[name="reject_reason"]');
      rejectSelectList.attr('enabled', 'true');
      $.each(reasonsList, function() {
        rejectSelectList.append(
          $("<option></option>").text(this.optionValue).val(this.optionValue).attr({'data-text': this.textareaValue, 'data-action': this.action})
        );
      });
      return false;
    });

    $(document).on('change', '.reject_form select[name="reject_reason"]', function(e){
      var id = $(this).closest('.reject_form').data('id');
      var selectedItem = this.options[e.target.selectedIndex];
      var textareaValue = $(selectedItem).data('text');
      var rejectAction = $(selectedItem).data('action');
      $('#reject_form_post_'+id).find("#reject_text_textarea").val(textareaValue);
      $('#reject_form_post_'+id).find('input[name="reject_action"]').val(rejectAction);
      if(rejectAction === ''){
        $('#reject_form_post_'+id+' .reject_reason').find(".error").text('Вы не выбрали причину отклонения публикации!');
      }
    });

    // перенести
    $('.post .infopanel input[name="transfer"]').live('click', function(){
      var id = $(this).data('id');
      $('#reject_form_post_'+id).addClass('hidden');
      $('#transfer_form_post_'+id).removeClass('hidden');
      return false;
    });

	// опубликовать
	$('.post .infopanel input[name="publish"]').live('click', function(){
		var id = $(this).data('id');
		$.post('/json/sandbox/publish/', { post_id: id }, function(json){
			if(json.messages=='ok'){
				$('#post_'+id).fadeOut(500,function(){
					$(this).remove();
				});
			}else{
				show_system_error(json);
			}
		},'json');
		return false;
	});

})
