/**
 * Скрипты для постов
 */

function change_post_subscibptions_checkboxes(json){
  if(typeof json.subscribe_mark !== undefined && json.subscribe_mark === '1' && $('#subscribe_comments').length){
    $('#subscribe_comments').attr('checked', true);
  }
  if(typeof json.tracker_mark !== undefined && json.tracker_mark === '1' && $('#tracker_comments').length){
    $('#tracker_comments').attr('checked', true);
  }
}

/**
 * Функция для обработки формы опроса в посте-вопросе.
 */
function posts_poll(form, button, action, target_id){
  $(form).ajaxSubmit({
    data: {ti: target_id, action: action},
    form: $(form),
    beforeSubmit: ajaxFormBeforSubmit,
    success: ajaxFormSuccess(function(json, statusText, xhr, jqForm){
      $(form).replaceWith(json.question_html);
    })
  });
}

/**
 * Функция для голосования за посты.
 */
function posts_vote(link, target_id, target_type, vote){
  $.post('/json/vote/',{ti:target_id, tt:target_type, v:vote},function(json){
    if(json.messages == 'ok'){
      var voting = $('#infopanel_post_'+target_id+' .js-voting');

      // выделим отмеченный пункт.
      if(vote === 1){
        voting.addClass('voted_plus').attr('title','Вы проголосовали положительно.');
        $('.js-plus',voting).attr('disabled', 'disabled');
        $('.js-minus',voting).attr('disabled', 'disabled');
      }else if(vote === -1){
        voting.addClass('voted_minus').attr('title','Вы проголосовали отрицательно.');
        $('.js-plus',voting).attr('disabled', 'disabled');
        $('.js-minus',voting).attr('disabled', 'disabled');
      }else{
        voting.addClass('voted');
        $('.js-plus',voting).attr('disabled', 'disabled');
        $('.js-minus',voting).attr('disabled', 'disabled');
      }

      // обновим кол-во голосов
      $('.js-score', voting).replaceWith('<span class="voting-wjt__counter-score js-score" title="Всего '+json.votes_count+': &uarr;'+json.votes_count_plus+' и &darr;'+json.votes_count_minus+'">'+json.score+'</span>');

      console.log(json.score);
      if(json.score !== '0'){
        // раскрасим positive / negative
        $('.js-mark', voting).addClass('voting-wjt__counter_'+json.sign);
      } else {
        $('.js-mark', voting).removeClass('voting-wjt__counter_positive voting-wjt__counter_negative');
      }

      // обновим строку с кол-вом доступных голосов в шапке сайта
      $('#charge_string').html(json.charge_string);
    }else{
      show_system_error(json);
    }
  },'json');
}

/**
 * Функция для добавления поста в избранное.
 */
function posts_add_to_favorite(link, target_type, target_id){

  var tags = $('#post_'+target_id+' .tags');
  var favs_count = $('#infopanel_post_'+target_id+' .js-favs_count');
  var count = favs_count.text() ? parseInt(favs_count.text()) : 0;
  var action = '';

  if($(link).hasClass('add')){ action = 'add';} else { action = 'remove';}

  $.post('/json/favorites/', {tt: target_type, ti: target_id, action: action}, function(json){
    if(json.messages == 'ok'){
      if(action == 'add'){
        $.jGrowl('Публикация добавлена в избранное', { });
        $(link).removeClass('add').addClass('remove');
        // теперь нужно показать кнопку "Изменить свои метки".
        if($('.favourites_edit_tags',tags).length === 0){
          tags.append('<li class="favourites_edit_tags"><a href="#" onclick="return show_edit_tags(this, \''+target_type+'\', '+target_id+');">изменить свои метки</a></li>');
        }
        favs_count.text(count + 1);
        change_post_subscibptions_checkboxes(json);
      }else{
        $.jGrowl('Публикация удалена из избранного', { });
        $(link).addClass('add').removeClass('remove');
        // удалим ссылку "Изменить свои метки".
        $('.favourites_edit_tags',tags).remove();
        $('#edit_tags_form').hide();
        var new_count = count - 1;

        favs_count.text(new_count ? new_count : '');
        $('#post_'+target_id+' .tags li.fav').remove();
      }
    }else{
      show_system_error(json);
    }
  },'json');
  return false;
}

/**
 * Функция для показа формы редактирования собственных тегов.
 */
function show_edit_tags(link, target_type, target_id){
  var edit_tags_form = $('#edit_tags_form');
  $('input[name="tt"]',edit_tags_form).val(target_type);
  $('input[name="ti"]',edit_tags_form).val(target_id);
  var tags = [];
  $('#post_'+target_id+' ul.tags li.fav a').each(function(){
    tags[tags.length] = $(this).text();
  });

  $('input[name="tags_string"]',edit_tags_form).val( tags.join(', ') ).focus();
  $('#post_'+target_id+' .tags').after(edit_tags_form);
  edit_tags_form.show();
  $('input[name="tags_string"]',edit_tags_form).focus();
  return false;
}

/**
 * Функция для закрытия формы редактирования собственных тегов.
 */
function close_edit_tags(){
  $('#edit_tags_form').hide();
  return false;
}

$(document).ready(function(){

  /* жалоба */
  if( $('.js-abuse_form').size() > 0 ){

    $('.js-abuse').on('click', function(){
      var parent = $(this).closest('.postinfo-panel__item_abuse');
      parent.addClass('abuse_huge');
      $('.js-abuse_form', parent).removeClass('hidden');
      $('.js-abuse', parent).addClass('hidden');
      $('.js-abuse_cancel', parent).removeClass('hidden');
      $('input[name="text"]', parent).focus();
    });

    $('.js-abuse_cancel').on('click', function(){
      var parent = $(this).closest('.postinfo-panel__item_abuse');
      parent.removeClass('abuse_huge');
      $('.js-abuse_form', parent).addClass('hidden');
      $('.js-abuse', parent).removeClass('hidden');
      $('.js-abuse_cancel', parent).addClass('hidden');
    });

    var input = $('#abuse_form input[name="text"]');
    input.bind('change keyup', function(){
      if( $(this).val() === '' ){
        $('#abuse_form button').attr('disabled', true);
      }else{
        $('#abuse_form button').attr('disabled', false);
      }
    });

    $('#abuse_form').ajaxForm({
        form: $('#abuse_form'),
        beforeSubmit: function(formData, jqForm, options){
          ajaxFormBeforSubmit(formData, jqForm, options);
          var text = $('#abuse_form input[name="text"]').val();
          var allSpacesRe = /\s+/g;
          text = text.replace(allSpacesRe, "");
          $('#abuse_form button').attr('disabled', true);
          if(text === ''){
            return false;
          }
        },
        success: ajaxFormSuccess(function(json, statusText, xhr, jqForm){
          $.jGrowl('Спасибо, жалоба отправлена.', { });
          $('#abuse_form').resetForm();
          $('#abuse_form button').attr('disabled', true);
          $('.js-abuse_cancel').click();
        }, false, true)
    });
  }

  // инициализируем автокомлпит для формы ввода пользовательских тегов
  $('#edit_tags_form .tags_string').autocomplete({
    serviceUrl: '/json/suggest/',
    minChars: 2,
    delimiter: /(,|;)\s*/, // Разделитель для нескольких запросов, символ или регулярное выражение
    maxHeight: 400,
    width: 300,
    zIndex: 9999,
    deferRequestBy: 500,
    params: { type: 'tags'}
  });

  // обработчик формы сохранения тегов.
  if($('#edit_tags_form').size()){
    $('#edit_tags_form').ajaxForm({
        form: $('#edit_tags_form'),
        beforeSubmit: ajaxFormBeforSubmit,
        success: ajaxFormSuccess(function(json, statusText, xhr, jqForm){
          var edit_tags_form = $('#edit_tags_form');
          var target_id = $('input[name="ti"]',edit_tags_form).val();
          var target_type = $('input[name="tt"]',edit_tags_form).val();
          $('#post_'+target_id+' .tags li.fav').remove();
          // вставим добавленные теги в список тегов к посту
          for(k in json.user_tags){
            $('#post_'+target_id+' .tags .favourites_edit_tags').before('<li class="fav">, <a rel="tag" href="/tag/'+json.user_tags[k]+'/">'+json.user_tags[k]+'</a></li>');
          }
          $('#edit_tags_form').hide();
        })
    });
  }

  $('#lenta_notifications .close').click(function(){
    $('#lenta_notifications').fadeOut();
    $.post('/json/notifications/hint_get_more_blogs/', function(json){
      if(json.messages == 'ok'){

      }else{
        show_system_error(json);
      }
    },'json');
    return false;
  });

  // Подписаться/Отписаться от компании

  $('.js-add_company_member').on('click', function(){
    var subscribeButton = $(this);
    var company_id = subscribeButton.data('company_id');
    $.post('/json/corporation/fan_add/', { company_id: company_id }, function(json){
      if(json.messages =='ok'){
        $('#company_'+company_id+' .js-remove_company_member').removeClass('hidden');
        $('#company_'+company_id+' .js-add_company_member').addClass('hidden');
      }else{
        show_system_error(json);
      }
    },'json');
  });

  $('.js-remove_company_member').on('click', function(){
    var subscribeButton = $(this);
    var company_id = subscribeButton.data('company_id');
    $.post('/json/corporation/fan_del/', { company_id: company_id }, function(json){
      if(json.messages =='ok'){
        $('#company_'+company_id+' .js-remove_company_member').addClass('hidden');
        $('#company_'+company_id+' .js-add_company_member').removeClass('hidden');
      }else{
        show_system_error(json);
      }
    },'json');
  });
});