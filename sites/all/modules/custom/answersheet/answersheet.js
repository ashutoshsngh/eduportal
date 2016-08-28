(function ($) {
    Drupal.behaviors.answersheet = {
        attach: function (context, settings) {
            $('#edit-select-paper').unbind().bind('change', function () {
                var selectedarray = this.value.split('-');
                $('#exam_code_value').text(selectedarray[4]);
                $('#exam_code_wrapper').show();
            });
        }
    };
})(jQuery);