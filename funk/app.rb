require 'json'
require 'date'
require 'slack-notifier'
require 'aws-sdk'

TABLE = [
  # 0     1     2     3     4     5     6     7     8     9
  [ '０', 'ワ', 'ヲ', 'ン', '゛', '゜', '６', '７', '８', '９' ], # 0
  [ 'Ｅ', 'ア', 'イ', 'ウ', 'エ', 'オ', 'Ａ', 'Ｂ', 'Ｃ', 'Ｄ' ], # 1
  [ 'Ｊ', 'カ', 'キ', 'ク', 'ケ', 'コ', 'Ｆ', 'Ｇ', 'Ｈ', 'Ｉ' ], # 2
  [ 'Ｏ', 'サ', 'シ', 'ス', 'セ', 'ソ', 'Ｋ', 'Ｌ', 'Ｍ', 'Ｌ' ], # 3
  [ 'Ｔ', 'タ', 'チ', 'ツ', 'テ', 'ト', 'Ｐ', 'Ｑ', 'Ｒ', 'Ｓ' ], # 4
  [ 'Ｙ', 'ナ', 'ニ', 'ヌ', 'ネ', 'ノ', 'Ｕ', 'Ｖ', 'Ｗ', 'Ｘ' ], # 5
  [ '／', 'ハ', 'ヒ', 'フ', 'ヘ', 'ホ', 'Ｚ', '？', '！', 'ー' ], # 6
  [ '　', 'マ', 'ミ', 'ム', 'メ', 'モ', '￥', '＆', '　', '　' ], # 7
  [ '　', 'ヤ', '（', 'ユ', '）', 'ヨ', '＊', '＃', '　', '　' ], # 8
  [ '５', 'ラ', 'リ', 'ル', 'レ', 'ロ', '１', '２', '３', '４' ], # 9
]

def lambda_handler(event:, context:)
  webhookurl = ENV["WEBHOOK_URL"]
  iotcoreurl = ENV["IOT_CORE_URL"]
  return unless webhookurl

  data = JSON.parse(event['payload'])['data'] if event['payload']
  message = data.chars.each_slice(2).map do |row, col|
    TABLE[row.to_i][col.to_i]
  end

  attachments = {
    attachments: [
      {
        color: '#154272',
        author_name: "Pokebell",
        author_icon: "https://user-images.githubusercontent.com/8074640/101222945-36791580-36ce-11eb-9d85-81a3ffed6547.png",
        title: "ポケベルからの発信です",
        text: message.join,
      }
    ]
  }

  notifier = Slack::Notifier.new webhookurl
  notifier.ping attachments
  
  client = Aws::IoTDataPlane::Client.new(endpoint: iotcoreurl)
  resp = client.publish({
    topic: "pokebell",
    qos: 1,
    payload: {message: message.join}.to_json,
  })

  {
    statusCode: 200,
    body: {
      message: "OK",
    }.to_json
  }
end
